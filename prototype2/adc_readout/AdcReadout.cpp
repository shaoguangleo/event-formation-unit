/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief ADC readout detector module.
 */

#include <iostream>
#include "AdcReadout.h"
#include "libs/include/Socket.h"

static struct AdcSettingsStruct {
  enum class PipelineType {RawData = 0, PeakFinder = 1};
  PipelineType Pipeline{PipelineType::RawData};
  enum class Polarity {Negative = 0, Positive = 1};
  Polarity PeakPolarity;
  std::uint16_t PeakDiscLow;
  std::uint16_t PeakDiscHigh;
} AdcSettings;

void SetCLIArguments(CLI::App &parser) {
//  parser.add_option("--pipeline", AdcSettings.Pipeline, "Select data processong pipeline (StoreData = 0, PeakFinder = 1).")->group("ADC Readout Options")->set_default_val("0");
//  parser.add_option("--peak_low_discriminator", AdcSettings.PeakDiscLow, "When using peak finding, ignores peaks that are smaller than this value above baseline.")->group("ADC Readout Options")->set_default_val("0");
//  parser.add_option("--peak_high_discriminator", AdcSettings.PeakDiscHigh, "When using peak finding, ignores peaks that are bigger than this value above baseline.")->group("ADC Readout Options")->set_default_val("65536");
  parser.add_option("--peak_polarity", AdcSettings.PeakPolarity, "Select expected peak polarity in relation to the baseline (Negative = 0, Positive = 1).")->group("ADC Readout Options")->set_default_val("1");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

ADC_Readout_Factory Factory;

AdcReadout::AdcReadout(BaseSettings Settings) : Detector("AdcReadout", Settings), toParsingQueue(100), ProducerPtr(new Producer(Settings.KafkaBroker, Settings.KafkaTopic)) {
  std::function<void()> inputFunc = [this](){AdcReadout::inputThread();};
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this](){AdcReadout::parsingThread();};
  Detector::AddThreadFunction(processingFunc, "parsing");
  Stats.setPrefix("adc_readout");
  Stats.create("input.bytes.received", AdcStats.input_bytes_received);
  Stats.create("parser.errors.unknown", AdcStats.parser_errors_unknown);
  Stats.create("parser.errors.feedf00d", AdcStats.parser_errors_feedf00d);
  Stats.create("parser.errors.filler", AdcStats.parser_errors_filler);
  Stats.create("parser.errors.beefcafe", AdcStats.parser_errors_beefcafe);
  Stats.create("parser.errors.dlength", AdcStats.parser_errors_dlength);
  Stats.create("parser.errors.abcd", AdcStats.parser_errors_abcd);
  Stats.create("parser.errors.hlength", AdcStats.parser_errors_hlength);
  Stats.create("parser.errors.type", AdcStats.parser_errors_type);
  Stats.create("parser.errors.ilength", AdcStats.parser_errors_ilength);
  Stats.create("parser.packets.total", AdcStats.parser_packets_total);
  Stats.create("parser.packets.idle", AdcStats.parser_packets_idle);
  Stats.create("parser.packets.data", AdcStats.parser_packets_data);
  Stats.create("parser.packets.error", AdcStats.parser_packets_error);
  Stats.create("processing.packets.lost", AdcStats.processing_packets_lost);
  AdcStats.processing_packets_lost = -1; //To compensate for the first error.
  
  Processor = std::unique_ptr<AdcDataProcessor>(new PeakFinder(ProducerPtr, bool(AdcSettings.PeakPolarity)));
  
//  switch (AdcSettings.Pipeline) {
//    case AdcSettingsStruct::PipelineType::RawData:
//      ProcessingFunction = {[this](PacketData &Data){rawData(Data);}};
//      break;
//    case AdcSettingsStruct::PipelineType::PeakFinder:
//      ProcessingFunction = {[this](PacketData &Data){peakFind(Data);}};
//      break;
//    default:
//      break;
//  }
}

void AdcReadout::inputThread() {
  std::uint64_t BytesReceived = 0;
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort);
  UDPServer mbdata(local);
  mbdata.setbuffers(0, 2000000);
  mbdata.printbuffers();
  mbdata.settimeout(0, 100000); // One tenth of a second
  ElementPtr DataElement;
  bool outCome;
  
  while (Detector::runThreads) {
    if (nullptr == DataElement) {
      outCome = toParsingQueue.waitGetEmpty(DataElement, 500);
      if (not outCome) {
        continue;
      }
    }
    DataElement->Length = mbdata.receive(static_cast<void*>(DataElement->Data), DataElement->MaxLength); //Fix cast
    if (DataElement->Length > 0) {
      BytesReceived += DataElement->Length;
      AdcStats.input_bytes_received = BytesReceived;
      toParsingQueue.tryPutData(std::move(DataElement));
    }
  }
}

void AdcReadout::addParserError(ParserException::Type ExceptionType) {
  switch (ExceptionType) {
    case ParserException::Type::UNKNOWN:
      ++AdcStats.parser_errors_unknown;
      break;
    case ParserException::Type::T_FEEDF00D:
      ++AdcStats.parser_errors_feedf00d;
      break;
    case ParserException::Type::T_0x55:
      ++AdcStats.parser_errors_filler;
      break;
    case ParserException::Type::D_BEEFCAFE:
      ++AdcStats.parser_errors_beefcafe;
      break;
    case ParserException::Type::D_LENGTH:
      ++AdcStats.parser_errors_dlength;
      break;
    case ParserException::Type::D_ABCD:
      ++AdcStats.parser_errors_abcd;
      break;
    case ParserException::Type::H_LENGTH:
      ++AdcStats.parser_errors_hlength;
      break;
    case ParserException::Type::H_TYPE:
      ++AdcStats.parser_errors_type;
      break;
    case ParserException::Type::I_LENGTH:
      ++AdcStats.parser_errors_ilength;
      break;
    default:
      ++AdcStats.parser_errors_unknown;
  }
}

void AdcReadout::parsingThread() {
  ElementPtr DataElement;
  bool GotElement = false;
  while (Detector::runThreads) {
    GotElement = toParsingQueue.waitGetData(DataElement, 1000);
    if (GotElement) {
      try {
        PacketData ParsedAdcData = parsePacket(*DataElement);
        if (ParsedAdcData.GlobalCount != ++LastGlobalCount) {
          ++AdcStats.processing_packets_lost;
          LastGlobalCount = ParsedAdcData.GlobalCount;
        }
        ++AdcStats.parser_packets_total;
        if (PacketType::Data == ParsedAdcData.Type) {
          ++AdcStats.parser_packets_data;
        } else if (PacketType::Idle == ParsedAdcData.Type) {
          ++AdcStats.parser_packets_idle;
        }
        (*Processor)(ParsedAdcData);
//        ProcessingFunction(ParsedAdcData);
      } catch (ParserException &e) {
        addParserError(e.getErrorType());
        ++AdcStats.parser_packets_error;
      }
      while (not toParsingQueue.tryPutEmpty(std::move(DataElement)) and Detector::runThreads) {
        //Do nothing
      }
    }
  }
}