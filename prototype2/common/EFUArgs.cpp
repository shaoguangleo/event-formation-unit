/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <cstdio>
#include <getopt.h>
#include <iostream>
#include <string>

EFUArgs *efu_args; /** global var */

EFUArgs::EFUArgs(int argc, char *argv[]) {
  using namespace std;

  optind = 1; // global variable used by getopt

  while (1) {
    static struct option long_options[] = {
        // clang-format-off
        {"help",      no_argument,       0, 'h'},
        {"broker",    required_argument, 0, 'b'},
        {"cpu",       required_argument, 0, 'c'},
        {"det",       required_argument, 0, 'd'},
        {"dip",       required_argument, 0, 'i'},
        {"dport",     required_argument, 0, 'p'},
        {"reports",   required_argument, 0, 'r'},
        {"stopafter", required_argument, 0, 's'},
        {"graphite",  required_argument, 0, 'g'},
        {"gport",     required_argument, 0, 'o'},
        {0, 0, 0, 0}};
        // clang-format_on

    int option_index = 0;

    int c =
        getopt_long(argc, argv, "b:c:d:g:o:i:p:r:s:h", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    // case 0: // currently not using flags
    //  if (long_options[option_index].flag != 0)
    //    break;
    case 'b':
      broker.assign(optarg);
      break;
    case 'c':
      cpustart = atoi(optarg);
      break;
    case 'd':
      det.assign(optarg);
      break;
    case 'g':
      graphite_ip_addr.assign(optarg);
      break;
    case 'o':
      graphite_port = atoi(optarg);
      break;
    case 'i':
      ip_addr.assign(optarg);
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'r':
      reportmask = (unsigned int)strtoul(optarg, 0, 0);
      break;
    case 's':
      stopafter = atoi(optarg);
      break;
    case 'h':
    default:
      printf("Usage: efu2 [OPTIONS]\n");
      printf(" --broker, -b broker     Kafka broker string \n");
      printf(" --cpu, -c lcore         lcore id of first thread \n");
      printf(" --det -d name           detector name \n");
      printf(" --dip, -i ipaddr        ip address of receive interface \n");
      printf(" --port -p port          udp port \n");
      printf(" --graphite, -g ipaddr   ip address of graphite metrics server \n");
      printf(" --gport -o port         Graphite tcp port \n");
      printf(" --stopafter, -s timeout terminate after timeout seconds \n");
      printf(" -h                      help - prints this message \n");
      stopafter = 0;
      return;
    }
  }

  cout << "Starting event processing pipeline2" << endl;
  cout << "  Detector:      " << det << endl;
  cout << "  CPU Offset:    " << cpustart << endl;
  cout << "  IP addr:       " << ip_addr << endl;
  cout << "  UDP Port:      " << port << endl;
  cout << "  Kafka broker:  " << broker << endl;
  cout << "  Graphite:      " << graphite_ip_addr << endl;
  cout << "  Graphite port: " << graphite_port << endl;
  cout << "  Stopafter:     " << stopafter << " seconds" << endl;
}