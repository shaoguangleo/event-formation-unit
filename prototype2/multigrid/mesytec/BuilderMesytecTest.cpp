/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mesytec/BuilderMesytec.h>
#include <multigrid/mesytec/TestData.h>
#include <multigrid/Config.h>
#include <tools/ReaderPcap.h>
#include <test/TestBase.h>

using namespace Multigrid;

class BuilderMesytecTest : public TestBase {
protected:
  Multigrid::Config config;
  
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }

  void load_config(std::string jsonfile) {
    config = Multigrid::Config(jsonfile);
  }
};

TEST_F(BuilderMesytecTest, ErrNoTimeStamp) {
  load_config(TEST_JSON_PATH "ILL_mappings.json");
  config.builder->parse(err_no_timestamp);

  EXPECT_EQ(config.builder->stats_discarded_bytes, 528);
  EXPECT_EQ(config.builder->stats_trigger_count, 1);
  EXPECT_EQ(config.builder->stats_bus_glitch_rejects, 0);
  EXPECT_EQ(config.builder->stats_readout_filter_rejects, 0);
  EXPECT_EQ(config.builder->stats_digital_geom_errors, 0);

  EXPECT_EQ(config.builder->ConvertedData.size(), 0);
}

TEST_F(BuilderMesytecTest, ParseRecordedWSData) {
  load_config(TEST_JSON_PATH "ILL_mappings.json");
  config.builder->parse(ws1);

  EXPECT_EQ(config.builder->stats_discarded_bytes, 0);
  EXPECT_EQ(config.builder->stats_trigger_count, 1);
  EXPECT_EQ(config.builder->stats_bus_glitch_rejects, 128);
  EXPECT_EQ(config.builder->stats_readout_filter_rejects, 0);
  EXPECT_EQ(config.builder->stats_digital_geom_errors, 0);

  EXPECT_EQ(config.builder->ConvertedData.size(), 0);
}

TEST_F(BuilderMesytecTest, ParseRecordedWSDataII) {
  load_config(TEST_JSON_PATH "ILL_mappings.json");
  config.builder->parse(ws2);

  EXPECT_EQ(config.builder->stats_discarded_bytes, 0);
  EXPECT_EQ(config.builder->stats_trigger_count, 2);
  EXPECT_EQ(config.builder->stats_bus_glitch_rejects, 256);
  EXPECT_EQ(config.builder->stats_readout_filter_rejects, 0);
  EXPECT_EQ(config.builder->stats_digital_geom_errors, 0);

  EXPECT_EQ(config.builder->ConvertedData.size(), 0);
}

TEST_F(BuilderMesytecTest, ParseRecordedWSDataIII) {
  load_config(TEST_JSON_PATH "ILL_mappings.json");
  config.builder->parse(ws3);

  EXPECT_EQ(config.builder->stats_discarded_bytes, 0);
  EXPECT_EQ(config.builder->stats_trigger_count, 4);
  EXPECT_EQ(config.builder->stats_bus_glitch_rejects, 256);
  EXPECT_EQ(config.builder->stats_readout_filter_rejects, 0);
  EXPECT_EQ(config.builder->stats_digital_geom_errors, 0);

  EXPECT_EQ(config.builder->ConvertedData.size(), 0);
}

TEST_F(BuilderMesytecTest, ParseRecordedWSDataMultipleTriggers) {
  load_config(TEST_JSON_PATH "ILL_mappings.json");
  config.builder->parse(ws4);

  EXPECT_EQ(config.builder->stats_discarded_bytes, 0);
  EXPECT_EQ(config.builder->stats_trigger_count, 36);
  EXPECT_EQ(config.builder->stats_bus_glitch_rejects, 0);
  EXPECT_EQ(config.builder->stats_readout_filter_rejects, 0);
  EXPECT_EQ(config.builder->stats_digital_geom_errors, 0);

  EXPECT_EQ(config.builder->ConvertedData.size(), 54);
}

TEST_F(BuilderMesytecTest, PCap2) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  ReaderPcap pcap(TEST_DATA_PATH "wireshark/Si_WHITE_1att_mvmeCrash.pcapng");

  uint8_t buffer[10000];
  int rdsize;
  size_t packets {0};
  while ((rdsize = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
    if (rdsize == 0) {
      continue; // non udp data
    }
    config.builder->parse(Buffer<uint8_t>(buffer, rdsize));
    packets++;
  }

  EXPECT_EQ(packets, 25);
  EXPECT_EQ(config.builder->stats_discarded_bytes, 3784);
  EXPECT_EQ(config.builder->stats_trigger_count, 0);
  EXPECT_EQ(config.builder->stats_bus_glitch_rejects, 0);
  EXPECT_EQ(config.builder->stats_readout_filter_rejects, 0);
  EXPECT_EQ(config.builder->stats_digital_geom_errors, 0);

  EXPECT_EQ(config.builder->ConvertedData.size(), 0);
}

TEST_F(BuilderMesytecTest, PCap3) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  ReaderPcap pcap(TEST_DATA_PATH "wireshark/endOfHFSeries_mvmeCrash.pcapng");

  uint8_t buffer[10000];
  int rdsize;
  size_t packets {0};
  while ((rdsize = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
    if (rdsize == 0) {
      continue; // non udp data
    }
    config.builder->parse(Buffer<uint8_t>(buffer, rdsize));
    packets++;
  }

  EXPECT_EQ(packets, 102);
  EXPECT_EQ(config.builder->stats_discarded_bytes, 12047);
  EXPECT_EQ(config.builder->stats_trigger_count, 0);
  EXPECT_EQ(config.builder->stats_bus_glitch_rejects, 0);
  EXPECT_EQ(config.builder->stats_readout_filter_rejects, 0);
  EXPECT_EQ(config.builder->stats_digital_geom_errors, 0);

  EXPECT_EQ(config.builder->ConvertedData.size(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
