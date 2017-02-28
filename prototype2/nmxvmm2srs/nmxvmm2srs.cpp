/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <NMX/ParserClusterer.h>
#include <nmxvmm2srs/NMXVMM2SRSData.h>
#include <cinttypes>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/NewStats.h>
#include <common/Producer.h>
#include <common/RingBuffer.h>
#include <common/Trace.h>
#include <cstring>
#include <iostream>
#include <libs/include/SPSCFifo.h>
#include <libs/include/Socket.h>
#include <libs/include/TSCTimer.h>
#include <libs/include/Timer.h>
#include <memory>
#include <stdio.h>
#include <unistd.h>

#define UNUSED __attribute__((unused))
#define ALIGN(x) __attribute__((aligned(x)))
//#define ALIGN(x)

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

using namespace std;
using namespace memory_sequential_consistent; // Lock free fifo

const char *classname = "NMX Detector for VMM2 data via SRS";

const int TSC_MHZ = 2900; // MJC's workstation - not reliable

/** ----------------------------------------------------- */

class NMXVMM2SRS : public Detector {
public:
  NMXVMM2SRS(void *args);
  void input_thread(void *args);
  void processing_thread(void *args);

  int statsize();
  int64_t statvalue(size_t index);
  std::string &statname(size_t index);

  /** @todo figure out the right size  of the .._max_entries  */
  static const int eth_buffer_max_entries = 20000;
  static const int eth_buffer_size = 9000;
  static const int kafka_buffer_size = 1000000;

private:
  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  char kafkabuffer[kafka_buffer_size];

  NewStats ns{"efu2.nmxvmm2srs."};

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo1_push_errors;
    int64_t pad[5];

    int64_t rx_idle1;
    int64_t rx_discards;
    int64_t rx_events;
  } ALIGN(64) mystats;

  EFUArgs *opts;
};

NMXVMM2SRS::NMXVMM2SRS(void *UNUSED args) {
  opts = (EFUArgs *)args;

  XTRACE(INIT, ALW, "Adding stats\n");
  // clang-format off
  ns.create("input.rx_packets",                &mystats.rx_packets);
  ns.create("input.rx_bytes",                  &mystats.rx_bytes);
  ns.create("input.dropped",                   &mystats.fifo1_push_errors);
  ns.create("processing.idle",                 &mystats.rx_idle1);
  // clang-format on

  XTRACE(INIT, ALW, "Creating %d NMX Rx ringbuffers of size %d\n",
         eth_buffer_max_entries, eth_buffer_size);
  eth_ringbuf = new RingBuffer<eth_buffer_size>(eth_buffer_max_entries);
  assert(eth_ringbuf != 0);
}

int NMXVMM2SRS::statsize() { return ns.size(); }

int64_t NMXVMM2SRS::statvalue(size_t index) { return ns.value(index); }

std::string &NMXVMM2SRS::statname(size_t index) { return ns.name(index); }

void NMXVMM2SRS::input_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;

  /** Connection setup */
  Socket::Endpoint local(opts->ip_addr.c_str(), opts->port);
  UDPServer nmxdata(local);
  nmxdata.buflen(opts->buflen);
  nmxdata.setbuffers(0, opts->rcvbuf);
  nmxdata.printbuffers();
  nmxdata.settimeout(0, 100000); // One tenth of a second

  int rdsize;
  Timer stop_timer;
  TSCTimer report_timer;
  for (;;) {
    unsigned int eth_index = eth_ringbuf->getindex();

    /** this is the processing step */
    if ((rdsize = nmxdata.receive(eth_ringbuf->getdatabuffer(eth_index),
                                  eth_ringbuf->getmaxbufsize())) > 0) {
      XTRACE(INPUT, DEB, "rdsize: %u\n", rdsize);
      mystats.rx_packets++;
      mystats.rx_bytes += rdsize;
      eth_ringbuf->setdatalength(eth_index, rdsize);

      if (input2proc_fifo.push(eth_index) == false) {
        mystats.fifo1_push_errors++;
      } else {
        eth_ringbuf->nextbuffer();
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

      if (stop_timer.timeus() >= opts->stopafter * 1000000LU) {
        std::cout << "stopping input thread, timeus " << stop_timer.timeus()
                  << std::endl;
        return;
      }
      report_timer.now();
    }
  }
}


void NMXVMM2SRS::processing_thread(void *args) {
  EFUArgs *opts = (EFUArgs *)args;
  assert(opts != NULL);

#ifndef NOKAFKA
  Producer producer(opts->broker, true, "NMX_detector");
#endif

  NMXVMM2SRSData data(4500);
  ParserClusterer parser;

  Timer stopafter_clock;
  TSCTimer report_timer;

  unsigned int data_index;
  int evtoff = 0;
  while (1) {
    if ((input2proc_fifo.pop(data_index)) == false) {
      mystats.rx_idle1++;
      usleep(10);
    } else {
      data.receive(eth_ringbuf->getdatabuffer(data_index),
                   eth_ringbuf->getdatalength(data_index));


      while (parser.event_ready()) {
        auto event = parser.get();
        event.analyze(true, 3, 7);
        if (event.good) {
          mystats.rx_events++;

          int time = 42; /**< @todo get time from ? */
          int pixelid = (int)event.x.center + (int)event.y.center * 256;

          std::memcpy(kafkabuffer + evtoff, &time, sizeof(time));
          std::memcpy(kafkabuffer + evtoff + 4, &pixelid, sizeof(pixelid));
          evtoff += 8;

          if (evtoff >= kafka_buffer_size / 10 - 20) {
            assert(evtoff < kafka_buffer_size);

#ifndef NOKAFKA
            producer.produce(kafkabuffer, evtoff);
            mystats.tx_bytes += evtoff;
#endif
            evtoff = 0;
          }
        } else {
          mystats.rx_discards +=
              event.x.entries.size() + event.y.entries.size();
        }
      }
    }

    // Checking for exit
    if (report_timer.timetsc() >= opts->updint * 1000000 * TSC_MHZ) {

      if (stopafter_clock.timeus() >= opts->stopafter * 1000000LU) {
        std::cout << "stopping processing thread, timeus " << std::endl;
        return;
      }
      report_timer.now();
    }
  }
}

/** ----------------------------------------------------- */

class NMXVMM2SRSFactory : DetectorFactory {
public:
  std::shared_ptr<Detector> create(void *args) {
    return std::shared_ptr<Detector>(new NMXVMM2SRS(args));
  }
};

NMXVMM2SRSFactory Factory;