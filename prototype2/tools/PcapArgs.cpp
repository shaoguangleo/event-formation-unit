/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <tools/PcapArgs.h>
#include <getopt.h>
#include <unistd.h>
// GCOVR_EXCL_START
PcapArgs::PcapArgs(int argc, char *argv[]) {
  while (1) {
    static struct option long_options[] = {
        {"filename", required_argument, 0, 'f'},
        {"ipaddr", required_argument, 0, 'i'},
        {"packets", required_argument, 0, 'a'},
        {"loop", no_argument, 0, 'l'},
        {"pcapoffset", no_argument, 0, 'o'},
        {"port", required_argument, 0, 'p'},
        {"throttle", required_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    int option_index = 0;

    int c = getopt_long(argc, argv, "a:f:o:i:lp:t:h", long_options, &option_index);
    if (c == -1)
      break;
    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0)
        break;
    case 'a':
      txPkt = atoi(optarg);
      break;
    case 'f':
      filename.assign(optarg);
      break;
    case 'i':
      dest_ip.assign(optarg);
      break;
    case 'o':
      pcapoffset = atoi(optarg);
      break;
    case 'l':
      loop = 1;
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 't':
      throttle = atoi(optarg);
      break;
    case 'h':
    default:
      printf("Usage: bulkdatagen [OPTIONS] \n");
      printf(" --filename -f name     read data from single file \n");
      printf(" --throttle -t val      speed throttle (0 fastest, then slower) "
             "\n");
      printf(" --packets -a number    number of packets to transmit \n");
      printf(" --ipaddr -i ipaddr     destination ip address \n");
      printf(" --loop -l              loop forever\n");
      printf(" --port -p port         UDP destination port \n");
      printf(" --throttle -t  val     sleep for (val) useconds after tx");
      printf(" -h                     help - prints this message \n");
      exit(1);
    }
  }
  printf("Generating a bulk data stream\n");
  if (!filename.empty())
  printf("  from file:              %s", filename.c_str());
  printf("  number of packets:      %" PRIu64 " packets\n", txPkt);
  printf("  starting at offset:     %d\n", pcapoffset);
  printf("  speed throttle:         %d\n", throttle);
  printf("Network properties\n");
  printf("  destination ip address: %s\n", dest_ip.c_str());
  printf("  destination udp port:   %d\n", port);
}
// GCOVR_EXCL_STOP
