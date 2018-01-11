/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief CSPEC Detector implementation
 */

#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/FBSerializer.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <cstring>
#include <efu/Parser.h>
#include <efu/Server.h>
#include <gdgem/nmx/HistSerializer.h>
#include <common/ReadoutSerializer.h>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <multigrid/mgmesytec/Data.h>
#include <queue>
#include <stdio.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_INF

using namespace memory_sequential_consistent; // Lock free fifo

const int TSC_MHZ = 2900; // Not accurate, do not rely solely on this

/** ----------------------------------------------------- */
struct DetectorSettingsStruct {
  uint32_t adcThreshold = {0}; // accept all, change to discard
  uint32_t wireThreshold = {0}; // accept all - @todo unused
  uint32_t gridThreshold = {0}; // accept all - @todo unused
} DetectorSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("--adc", DetectorSettings.adcThreshold,
         "minimum adc value for accept")->group("MGMesytec");
  parser.add_option("--wire", DetectorSettings.wireThreshold,
         "minimum wire adc value for accept")->group("MGMesytec");
  parser.add_option("--grid", DetectorSettings.gridThreshold,
         "minimum grid adc value for accept")->group("MGMesytec");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

/** ----------------------------------------------------- */
const char *classname = "CSPEC Detector Mesytec readout";

class CSPEC : public Detector {
public:
  CSPEC(BaseSettings settings);
  void input_thread();
  void processing_thread();

  const char *detectorname();

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 1000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 1000000;

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo_push_errors;
    int64_t pad_a[5]; /**< @todo check alignment*/

    // Processing Counters
    int64_t rx_readouts;
    int64_t rx_error_bytes;
    int64_t rx_discards;
    int64_t rx_idle1;
    int64_t geometry_errors;
    int64_t fifo_seq_errors;
    // Output Counters
    int64_t rx_events;
    int64_t tx_bytes;
  } ALIGN(64) mystats;
};

CSPEC::CSPEC(BaseSettings settings) : Detector(settings) {
  Stats.setPrefix("efu2.mgmesytec");

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  Stats.create("rx_packets",           mystats.rx_packets);
  Stats.create("rx_bytes",             mystats.rx_bytes);
  Stats.create("i2pfifo_dropped",      mystats.fifo_push_errors);
  Stats.create("readouts",             mystats.rx_readouts);
  Stats.create("readouts_error_bytes", mystats.rx_error_bytes);
  Stats.create("readouts_discarded",   mystats.rx_discards);
  Stats.create("processing_idle",      mystats.rx_idle1);
  Stats.create("geometry_errors",      mystats.geometry_errors);
  Stats.create("fifo_seq_errors",      mystats.fifo_seq_errors);
  Stats.create("events",               mystats.rx_events);
  Stats.create("tx_bytes",             mystats.tx_bytes);
  // clang-format on

  std::function<void()> inputFunc = [this]() { CSPEC::input_thread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this]() {
    CSPEC::processing_thread();
  };
  Detector::AddThreadFunction(processingFunc, "processing");

  XTRACE(INIT, ALW, "Creating %d Ethernet ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries + 11);
}

const char *CSPEC::detectorname() { return classname; }

void CSPEC::input_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort);
  UDPServer cspecdata(local);
  cspecdata.setbuffers(EFUSettings.DetectorTxBufferSize, EFUSettings.DetectorRxBufferSize);
  cspecdata.printbuffers();
  cspecdata.settimeout(0, 100000); // One tenth of a second

  int rdsize;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getindex();

    /** this is the processing step */
    eth_ringbuf->setdatalength(eth_index, 0);
    if ((rdsize = cspecdata.receive(eth_ringbuf->getdatabuffer(eth_index),
                                    eth_ringbuf->getmaxbufsize())) > 0) {
      eth_ringbuf->setdatalength(eth_index, rdsize);
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      XTRACE(INPUT, DEB, "rdsize: %u\n", rdsize);

      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo_push_errors++;
      } else {
        eth_ringbuf->nextbuffer();
      }
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping input thread.\n");
      return;
    }
  }
}

void CSPEC::processing_thread() {
  Producer producer(EFUSettings.KafkaBroker, "C-SPEC_detector");
  Producer monitorprod(EFUSettings.KafkaBroker, "C-SPEC_monitor");
  FBSerializer flatbuffer(kafka_buffer_size, producer);
  ReadoutSerializer readouts(100000, monitorprod);
  HistSerializer histfb;
  NMXHists hists;

  MesytecData dat;

  dat.setAdcThreshold(DetectorSettings.adcThreshold);
  dat.setWireThreshold(DetectorSettings.adcThreshold);
  dat.setGridThreshold(DetectorSettings.adcThreshold);

  TSCTimer report_timer;
  TSCTimer timestamp;

  unsigned int data_index;
  while (1) {

    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      usleep(1);
    } else {
      auto len = eth_ringbuf->getdatalength(data_index);
      if (len == 0) {
        mystats.fifo_seq_errors++;
      } else {
        dat.parse(eth_ringbuf->getdatabuffer(data_index),
                  eth_ringbuf->getdatalength(data_index), hists, readouts);

        mystats.tx_bytes += flatbuffer.addevent(42, 1);
        mystats.rx_events++;
        mystats.rx_readouts += dat.readouts;
        mystats.rx_discards += dat.discards;
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >=
        EFUSettings.UpdateIntervalSec * 1000000 * TSC_MHZ) {
      mystats.tx_bytes += flatbuffer.produce();

      readouts.produce();

      if (hists.empty()) { /**< @todo wrong logic ? */
        XTRACE(PROCESS, DEB, "Sending histogram for %zu readouts\n", hists.eventlet_count());
        char *txbuffer;
        auto len = histfb.serialize(hists, &txbuffer);
        monitorprod.produce(txbuffer, len);
        hists.clear();
      }

      report_timer.now();
    }

    // Checking for exit
    if (not runThreads) {
      XTRACE(INPUT, ALW, "Stopping processing thread.\n");
      return;
    }
  }
}

/** ----------------------------------------------------- */

class CSPECFactory : public DetectorFactory {
public:
  std::shared_ptr<Detector> create(BaseSettings settings) {
    return std::shared_ptr<Detector>(new CSPEC(settings));
  }
};

CSPECFactory Factory;
