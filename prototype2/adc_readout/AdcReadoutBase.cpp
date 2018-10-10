/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief ADC readout detector module.
 */

#include "AdcReadoutBase.h"
#include "AdcSettings.h"
#include "PeakFinder.h"
#include "SampleProcessing.h"
#include "UDPClient.h"
#include <common/Log.h>
#include <memory>

AdcReadoutBase::AdcReadoutBase(BaseSettings const &Settings,
                               AdcSettings &ReadoutSettings)
    : Detector("AdcReadout", Settings), ReadoutSettings(ReadoutSettings),
      GeneralSettings(Settings), Service(std::make_shared<asio::io_service>()),
      Worker(*Service.get()) {
  // Note: Must call this->inputThread() instead of
  // AdcReadoutBase::inputThread() for the unit tests to work
  std::function<void()> inputFunc = [this]() { this->inputThread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  Stats.setPrefix("adc_readout" + ReadoutSettings.GrafanaNameSuffix);
  Stats.create("input.bytes.received", AdcStats.input_bytes_received);
  Stats.create("parser.errors", AdcStats.parser_errors);
  Stats.create("parser.unknown_channel", AdcStats.parser_unknown_channel);
  Stats.create("parser.packets.total", AdcStats.parser_packets_total);
  Stats.create("parser.packets.idle", AdcStats.parser_packets_idle);
  Stats.create("parser.packets.data", AdcStats.parser_packets_data);
  Stats.create("processing.packets.lost", AdcStats.processing_packets_lost);
  Stats.create("current_ts", AdcStats.current_ts_sec);
  Stats.create("current_ts_alt", AdcStats.current_ts_alt_sec);
  AdcStats.processing_packets_lost = -1; // To compensate for the first error.
}

std::shared_ptr<Producer> AdcReadoutBase::getProducer() {
  if (ProducerPtr == nullptr) {
    ProducerPtr = std::shared_ptr<Producer>(
        new Producer(GeneralSettings.KafkaBroker, GeneralSettings.KafkaTopic));
  }
  return ProducerPtr;
}

void AdcReadoutBase::stopThreads() {
  Service->stop();
  Detector::stopThreads();
}

SamplingRun *AdcReadoutBase::GetDataModule(ChannelID const Identifier) {
  if (DataModuleQueues.find(Identifier) == DataModuleQueues.end()) {
    DataModuleQueues.emplace(
        std::make_pair(Identifier, std::make_unique<Queue>(MessageQueueSize)));
    auto ProcessingID = DataModuleQueues.size() - 1;
    auto ThreadName = "processing_" + std::to_string(ProcessingID);
    std::function<void()> processingFunc = [this, ThreadName, Identifier]() {
      this->processingThread(*this->DataModuleQueues.at(Identifier));
    };
    Detector::AddThreadFunction(processingFunc, ThreadName);
    auto &NewThread = Detector::Threads.at(Detector::Threads.size() - 1);
    NewThread.thread = std::thread(NewThread.func);
  }
  SpscBuffer::ElementPtr<SamplingRun> ReturnModule{nullptr};
  bool Success = DataModuleQueues.at(Identifier)->tryGetEmpty(ReturnModule);
  if (Success) {
    return ReturnModule;
  }
  return nullptr;
}

bool AdcReadoutBase::QueueUpDataModule(SamplingRun *Data) {
  return DataModuleQueues.at(Data->Identifier)
      ->tryPutData(SpscBuffer::ElementPtr<SamplingRun>(Data));
}

void AdcReadoutBase::packetFunction(InData const &Packet,
                                    PacketParser &Parser) {
  if (Packet.Length > 0) {
    AdcStats.input_bytes_received += Packet.Length;
    try {
      try {
        auto PacketInfo = Parser.parsePacket(Packet);
        if (PacketInfo.GlobalCount != ++LastGlobalCount) {
          ++AdcStats.processing_packets_lost;
          LastGlobalCount = PacketInfo.GlobalCount;
        }
        ++AdcStats.parser_packets_total;
        if (PacketType::Data == PacketInfo.Type) {
          ++AdcStats.parser_packets_data;
        } else if (PacketType::Idle == PacketInfo.Type) {
          ++AdcStats.parser_packets_idle;
        }
      } catch (ParserException &e) {
        ++AdcStats.parser_errors;
      }
    } catch (ModuleProcessingException &E) {
      AdcStats.processing_buffer_full++;
      while (Detector::runThreads and
             not QueueUpDataModule(std::move(E.UnproccesedData))) {
      }
    }
  }
}

void AdcReadoutBase::inputThread() {
  std::vector<std::unique_ptr<UDPClient>> UDPReceivers;

  std::function<SamplingRun *(ChannelID)> DataModuleProducer(
      [this](auto Identifier) { return this->GetDataModule(Identifier); });

  std::function<bool(SamplingRun *)> QueingFunction1([this](SamplingRun *Data) {
    this->AdcStats.current_ts_sec = Data->TimeStamp.Seconds;
    return this->QueueUpDataModule(Data);
  });

  PacketParser Parser1(QueingFunction1, DataModuleProducer, 0);
  UDPReceivers.emplace_back(std::make_unique<UDPClient>(
      Service, EFUSettings.DetectorAddress, EFUSettings.DetectorPort,
      [&Parser1, this](auto Packet) {
        this->packetFunction(Packet, Parser1);
      }));

  std::function<bool(SamplingRun *)> QueingFunction2([this](SamplingRun *Data) {
    this->AdcStats.current_ts_alt_sec = Data->TimeStamp.Seconds;
    return this->QueueUpDataModule(Data);
  });

  PacketParser Parser2(QueingFunction2, DataModuleProducer, 1);
  if (not ReadoutSettings.AltDetectorInterface.empty() and
      ReadoutSettings.AltDetectorPort != 0) {
    UDPReceivers.emplace_back(std::make_unique<UDPClient>(
        Service, ReadoutSettings.AltDetectorInterface,
        ReadoutSettings.AltDetectorPort, [&Parser2, this](auto Packet) {
          this->packetFunction(Packet, Parser2);
        }));
  }
  Service->run();
}

void AdcReadoutBase::processingThread(Queue &DataModuleQueue) {
  std::vector<std::unique_ptr<AdcDataProcessor>> Processors;

  if (ReadoutSettings.PeakDetection) {
    Processors.push_back(
        std::unique_ptr<AdcDataProcessor>(new PeakFinder(getProducer())));
  }
  if (ReadoutSettings.SerializeSamples) {
    std::unique_ptr<AdcDataProcessor> Processor(
        new SampleProcessing(getProducer(), ReadoutSettings.Name));
    dynamic_cast<SampleProcessing *>(Processor.get())
        ->setTimeStampLocation(
            TimeStampLocationMap.at(ReadoutSettings.TimeStampLocation));
    dynamic_cast<SampleProcessing *>(Processor.get())
        ->setMeanOfSamples(ReadoutSettings.TakeMeanOfNrOfSamples);
    dynamic_cast<SampleProcessing *>(Processor.get())
        ->setSerializeTimestamps(ReadoutSettings.SampleTimeStamp);
    Processors.emplace_back(std::move(Processor));
  }

  bool GotModule = false;
  DataModulePtr Data = nullptr;
  const std::int64_t TimeoutUSecs = 1000000;
  while (Detector::runThreads) {
    GotModule = DataModuleQueue.waitGetData(Data, TimeoutUSecs);
    if (GotModule) {
      try {
        for (auto &Processor : Processors) {
          (*Processor).processData(*Data);
        }
      } catch (ParserException &e) {
        ++AdcStats.parser_errors;
      }
      while (not DataModuleQueue.tryPutEmpty(std::move(Data)) and
             Detector::runThreads) {
        // Do nothing
      }
    }
  }
}
