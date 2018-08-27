/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Processing pipeline for CSPEC instrument (Multi-Grid detector using
/// Mesytec readout)
///
//===----------------------------------------------------------------------===//

#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/TimeString.h>
#include <cstring>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <common/HistSerializer.h>
#include <common/ReadoutSerializer.h>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <multigrid/mgmesytec/Sis3153Parser.h>
#include <multigrid/mgmesytec/Vmmr16Parser.h>
#include <multigrid/MgConfig.h>
#include <multigrid/mgmesytec/HitFile.h>
#include <queue>
#include <stdio.h>
#include <unistd.h>

#include <multigrid/mgmesytec/SequoiaGeometry.h>
//#include <multigrid/mgmesytec/MG24Geometry.h>

#include <multigrid/mgmesytec/EfuMaximum.h>
#include <multigrid/mgmesytec/EfuCenterMass.h>
#include <multigrid/mgmesytec/EfuPrioritized.h>

#include <common/Log.h>

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this

/** ----------------------------------------------------- */
struct DetectorSettingsStruct {
  std::string ConfigFile;
  bool monitor{false};
  std::string fileprefix{""};
} DetectorSettings;

void SetCLIArguments(CLI::App & parser) {
  parser.add_option("-f,--file", DetectorSettings.ConfigFile,
                    "NMX (gdgem) specific config file")->group("MGMesytec")->
                    required()->configurable(true);
  parser.add_flag("--monitor", DetectorSettings.monitor,
                  "stream monitor data")->group("MGMesytec")->configurable(true)->set_default_val("true");
  parser.add_option("--dumptofile", DetectorSettings.fileprefix,
                    "dump to specified file")->group("MGMesytec")->configurable(true);
}

PopulateCLIParser PopulateParser{SetCLIArguments};

struct Monitor
{
  std::shared_ptr<Hists> hists;
  std::shared_ptr<ReadoutSerializer> readouts;

  void init(std::string broker, size_t max_readouts) {
    readouts = std::make_shared<ReadoutSerializer>(max_readouts);
    hists = std::make_shared<Hists>(std::numeric_limits<uint16_t>::max(),
                                    std::numeric_limits<uint16_t>::max());
    histfb = std::make_shared<HistSerializer>(hists->needed_buffer_size());

    producer = std::make_shared<Producer>(broker, "C-SPEC_monitor");
    readouts->set_callback(std::bind(&Producer::produce2<uint8_t>, producer.get(), std::placeholders::_1));
    histfb->set_callback(std::bind(&Producer::produce2<uint8_t>, producer.get(), std::placeholders::_1));
    enabled_ = true;
  }

  void close() {
    enabled_ = false;
    hists.reset();
    readouts.reset();
    histfb.reset();
    producer.reset();
  }

  void produce() {
    if (!enabled_)
      return;

    if (!hists->isEmpty()) {
      LOG(Sev::Debug, "Flushing histograms for {} readouts", hists->hit_count());
      histfb->produce(*hists);
      hists->clear();
    }

    if (readouts->getNumEntries()) {
      LOG(Sev::Debug, "Flushing readout data for {} readouts", readouts->getNumEntries());
      readouts->produce();
    }
  }

  bool enabled() const {
    return enabled_;
  }

  Monitor() = default;
  ~Monitor() { close(); }

private:
  bool enabled_ {false};

  std::shared_ptr<Producer> producer;
  std::shared_ptr<HistSerializer> histfb;
};

/** ----------------------------------------------------- */
const char *classname = "CSPEC Detector Mesytec readout";

///
class CSPEC : public Detector {
public:
  CSPEC(BaseSettings settings);
  void mainThread();

  const char *detectorname();

  /// Some hardcoded constants
  static constexpr int eth_buffer_size = 9000;          /// used for experimentation
  const int kafka_buffer_size = 1000000;     /// -||-
  const int readout_entries = 100000;        /// number of raw readout entries
  const int one_tenth_second_usecs = 100000; ///

private:

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t rx_discarded_bytes;
    int64_t triggers;
    int64_t badtriggers;
    int64_t rx_readouts;
    int64_t parse_errors;
    int64_t rx_discards;
    int64_t geometry_errors;
    int64_t timing_errors;
    int64_t rx_events;
    int64_t tx_bytes;
    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails;
    int64_t kafka_ev_errors;
    int64_t kafka_ev_others;
    int64_t kafka_dr_errors;
    int64_t kafka_dr_noerrors;
  } ALIGN(64) mystats;

  void init_config();

  Multigrid::Config mg_config;
  Monitor monitor;

  uint64_t RecentPulseTime{0};

  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};

  Multigrid::VMMR16Parser vmmr16Parser;

  std::shared_ptr<Multigrid::Efu> mgEfu;
  std::shared_ptr<Multigrid::HitFile> dumpfile;

  uint32_t getPixel() {
    if (!mgEfu)
      return 0;
    return mg_config.geometry.pixel3D(mgEfu->x(),
                            mgEfu->y(),
                            mgEfu->z());
  }

  uint32_t getTime() {
    return static_cast<uint32_t>(mgEfu->time() - RecentPulseTime);
  }
};

CSPEC::CSPEC(BaseSettings settings) : Detector("CSPEC", settings) {
  Stats.setPrefix("efu.mgmesytec");

  LOG(Sev::Info, "Adding stats");
  // clang-format off
  Stats.create("rx_packets",            mystats.rx_packets);
  Stats.create("rx_bytes",              mystats.rx_bytes);
  Stats.create("rx_discarded_bytes",    mystats.rx_discarded_bytes);
  Stats.create("readouts",              mystats.rx_readouts);
  Stats.create("triggers",              mystats.triggers);
  Stats.create("badtriggers",           mystats.badtriggers);
  Stats.create("readouts_parse_errors", mystats.parse_errors);
  Stats.create("readouts_discarded",    mystats.rx_discards);
  Stats.create("geometry_errors",       mystats.geometry_errors);
  Stats.create("timing_errors",         mystats.timing_errors);
  Stats.create("events",                mystats.rx_events);
  Stats.create("tx_bytes",              mystats.tx_bytes);
  /// Todo below stats are common to all detectors and could/should be moved
  Stats.create("kafka_produce_fails", mystats.kafka_produce_fails);
  Stats.create("kafka_ev_errors", mystats.kafka_ev_errors);
  Stats.create("kafka_ev_others", mystats.kafka_ev_others);
  Stats.create("kafka_dr_errors", mystats.kafka_dr_errors);
  Stats.create("kafka_dr_others", mystats.kafka_dr_noerrors);
  // clang-format on

  std::function<void()> inputFunc = [this]() { CSPEC::mainThread(); };
  Detector::AddThreadFunction(inputFunc, "main");

  LOG(Sev::Info, "Stream monitor data = {}",
      (DetectorSettings.monitor ? "YES" : "no"));
  if (!DetectorSettings.fileprefix.empty())
    LOG(Sev::Info, "Dump h5 data in path: {}",
           DetectorSettings.fileprefix);
}

const char *CSPEC::detectorname() { return classname; }



void CSPEC::init_config()
{
  LOG(Sev::Info, "MG Config file: {}", DetectorSettings.ConfigFile);
  mg_config = Multigrid::Config(DetectorSettings.ConfigFile);
  LOG(Sev::Info, "Multigrid Config\n{}", mg_config.debug());

  if (DetectorSettings.monitor)
    monitor.init(EFUSettings.KafkaBroker, readout_entries);

  if (mg_config.reduction_strategy == "center-mass") {
    mgEfu = std::make_shared<Multigrid::EfuCenterMass>();
  } else if (mg_config.reduction_strategy == "prioritized") {
    mgEfu = std::make_shared<Multigrid::EfuPrioritized>();
  } else {
    mgEfu = std::make_shared<Multigrid::EfuMaximum>();
  }
  mgEfu->mappings = mg_config.mappings;
  mgEfu->hists = monitor.hists;
  mgEfu->raw1 = monitor.readouts;

  if (!DetectorSettings.fileprefix.empty())
  {
    dumpfile = Multigrid::HitFile::create(
        DetectorSettings.fileprefix + "mgmesytec_" + timeString(), 100);
  }
  vmmr16Parser.spoof_high_time(mg_config.spoof_high_time);
}

void CSPEC::mainThread() {
  init_config();

  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort); //Change name or add more comments
  UDPReceiver cspecdata(local);
  cspecdata.setBufferSizes(EFUSettings.DetectorTxBufferSize, EFUSettings.DetectorRxBufferSize);
  cspecdata.printBufferSizes();
  cspecdata.setRecvTimeout(0, one_tenth_second_usecs); /// secs, usecs

  EV42Serializer ev42serializer(kafka_buffer_size, "multigrid");
  Producer EventProducer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  ev42serializer.set_callback(std::bind(&Producer::produce2<uint8_t>, &EventProducer, std::placeholders::_1));

  Multigrid::Sis3153Parser sis3153parser;
  sis3153parser.buffers.reserve(1000);

  uint8_t buffer[eth_buffer_size];
  size_t ReadSize {0};
  TSCTimer report_timer;
  for (;;) {
    if ((ReadSize = cspecdata.receive(buffer, eth_buffer_size)) > 0) {
      mystats.rx_packets++;
      mystats.rx_bytes += ReadSize;
      LOG(Sev::Debug, "Processed UDP packed of size: {}", ReadSize);

      mystats.rx_discarded_bytes += sis3153parser.parse(Buffer<uint8_t>(buffer, ReadSize));
      if (sis3153parser.buffers.empty()) {
        mystats.parse_errors++;
      }

      for (const auto &b : sis3153parser.buffers) {

        mystats.rx_discarded_bytes += vmmr16Parser.parse(b);

        if (vmmr16Parser.converted_data.empty())
          continue;

        if (dumpfile) {
          dumpfile->push(vmmr16Parser.converted_data);
        }

        auto parsed_readouts = vmmr16Parser.converted_data.size();

        if (vmmr16Parser.externalTrigger()) {
          parsed_readouts--;
          ev42serializer.set_pulse_time(RecentPulseTime);
          if (ev42serializer.events())
            mystats.tx_bytes += ev42serializer.produce();
          if (RecentPulseTime) {
            auto PulsePeriod = vmmr16Parser.time() - RecentPulseTime;
            ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
          }
          RecentPulseTime = vmmr16Parser.time();
        }

        mystats.triggers = vmmr16Parser.trigger_count();
        mystats.rx_readouts += parsed_readouts;

        if (mgEfu) {
          mgEfu->ingest(vmmr16Parser.converted_data);
//stats.discards++;

          if (mgEfu->event_good()) {
            uint32_t pixel = getPixel();
            uint32_t time = getTime();

            XTRACE(PROCESS, DEB, "Event: pixel: %d, time: %d ", pixel, time);
            if (pixel == 0) {
              mystats.geometry_errors++;
            } else if (time > (1.00004 * ShortestPulsePeriod)) {
              mystats.timing_errors++;
            } else {
              mystats.tx_bytes += ev42serializer.addevent(time, pixel);
              mystats.rx_events++;
            }
          } else {
// \todo external triggers treated as "bad"?
            mystats.badtriggers++;
          }
        }
      }
    }

    // Force periodic flushing
    if (report_timer.timetsc() >= EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {
      mystats.tx_bytes += ev42serializer.produce();
      monitor.produce();
      report_timer.now();
    }

    // Checking for exit
    if (not runThreads) {
      // flush anything that remains
      mystats.tx_bytes += ev42serializer.produce();
      monitor.produce();
      LOG(Sev::Info, "Stopping processing thread.");
      return;
    }
  }
}

/** ----------------------------------------------------- */

DetectorFactory<CSPEC> Factory;
