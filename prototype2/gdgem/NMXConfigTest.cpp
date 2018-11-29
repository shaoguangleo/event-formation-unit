/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <common/DataSave.h>
#include <cstring>
#include <gdgem/NMXConfig.h>
#include <memory>
#include <test/TestBase.h>
#include <unistd.h>

std::string pathprefix{""};

std::string nocalibration{""};

using namespace Gem;

class NMXConfigTest : public TestBase {
protected:
  virtual void SetUp() {
  }

  virtual void TearDown() { }
};

/// \todo modernize this
bool cwdContains(const char * searchfor) {
  char cwdname[1024];
  auto __attribute__((unused)) retval = getcwd(cwdname, sizeof(cwdname));
  auto rt = strstr(cwdname, searchfor);
  return (rt != NULL);
}

/** Test cases below */
TEST_F(NMXConfigTest, ConstructorDefaults) {
  NMXConfig nmxconfig;
  ASSERT_EQ("VMM3", nmxconfig.builder_type);
  ASSERT_EQ(nmxconfig.calfile, nullptr);
}

TEST_F(NMXConfigTest, EventFilter) {
  EventFilter filter;
  Event e; // use empty Event
  filter.enforce_lower_uncertainty_limit = false;
  filter.enforce_minimum_hits = false;
  ASSERT_TRUE(filter.valid(e, utpcResults()));

  filter.enforce_lower_uncertainty_limit = false;
  filter.enforce_minimum_hits = true;
  ASSERT_FALSE(filter.valid(e, utpcResults()));

  /// \todo test this behaviour
  // filter.enforce_lower_uncertainty_limit = true;
  // filter.enforce_minimum_hits = false;
  // ASSERT_FALSE(filter.valid(e));
}


TEST_F(NMXConfigTest, NoConfigFile) {
  NMXConfig nmxconfig("file_does_not_exist", nocalibration);
  ASSERT_EQ("VMM3", nmxconfig.builder_type);
  // ASSERT_EQ(256, nmxconfig.geometry_x);
  // ASSERT_EQ(256, nmxconfig.geometry_y);
}

TEST_F(NMXConfigTest, DebugPrint) {
  MESSAGE() << "This is Not a test, but simply exercises the debug print code" << "\n";
  NMXConfig nmxconfig;
  nmxconfig.filter.enforce_lower_uncertainty_limit = true;
  nmxconfig.filter.enforce_minimum_hits = true;
  auto str = nmxconfig.debug();
  MESSAGE() << str << "\n";
}

TEST_F(NMXConfigTest, JsonConfig) {
  NMXConfig nmxconfig(pathprefix + "../prototype2/gdgem/configs/vmm3.json", nocalibration);
  ASSERT_EQ(100, nmxconfig.time_config.tac_slope()); // Parsed from json
  ASSERT_EQ(20, nmxconfig.time_config.bc_clock());
}

int main(int argc, char **argv) {
  // Assume root is build/ directory - for running manually
  // but check for VM builds of Linux and MacOS
  if (cwdContains("build/prototype2")) { //Linux
  // Assume we're in prototype2/build/prototype2/gdgem
    pathprefix = "../../../build/";
  }
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
