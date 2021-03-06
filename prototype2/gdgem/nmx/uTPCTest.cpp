/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/uTPC.h>
#include <cmath>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

using namespace Gem;

class uTPCTest : public TestBase {
protected:
  Hit hit;
  Cluster cluster;
  Event event;
  virtual void SetUp() { }
  virtual void TearDown() { }
};

TEST_F(uTPCTest, AnalyzeInvalid) {
  auto result = utpcAnalyzer(false, 2, 2).analyze(cluster);
  EXPECT_TRUE(std::isnan(result.utpc_center));
}

TEST_F(uTPCTest, AnalyzeAverage) {
  Hit hit;
  hit.coordinate = 0;
  hit.weight = 2;
  cluster.insert(hit);
  auto result = utpcAnalyzer(false, 1, 1).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 0);
  hit.coordinate = 1;
  hit.weight = 4;
  cluster.insert(hit);
  hit.coordinate = 2;
  hit.weight = 4;
  cluster.insert(hit);
  result = utpcAnalyzer(false, 1, 1).analyze(cluster);
  EXPECT_EQ(cluster.hit_count(), 3);
  EXPECT_EQ(result.utpc_center, 1);
  result = utpcAnalyzer(true, 1, 1).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 1.2);
  EXPECT_EQ(result.utpc_center_rounded(), 1);
}

TEST_F(uTPCTest, AnalyzeUncert) {
  hit.weight = 1;

  hit.time = hit.coordinate = 0;
  cluster.insert(hit);
  hit.time = hit.coordinate = 1;
  cluster.insert(hit);
  hit.time = hit.coordinate = 2;
  cluster.insert(hit);

  auto result = utpcAnalyzer(true, 1, 1).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 2);
  EXPECT_EQ(result.uncert_lower, 1);
  EXPECT_EQ(result.uncert_upper, 1);

  result = utpcAnalyzer(true, 2, 2).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 2);
  EXPECT_EQ(result.uncert_lower, 1);
  EXPECT_EQ(result.uncert_upper, 2);

  hit.coordinate = 31;
  cluster.insert(hit);
  result = utpcAnalyzer(true, 2, 2).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 16.5);
  EXPECT_EQ(result.uncert_lower, 30);
  EXPECT_EQ(result.uncert_upper, 31);

  result = utpcAnalyzer(true, 5, 5).analyze(cluster);
  EXPECT_EQ(result.utpc_center, 16.5);
  EXPECT_EQ(result.uncert_lower, 30);
  EXPECT_EQ(result.uncert_upper, 32);

  EXPECT_EQ(result.utpc_center, 16.5);
  EXPECT_EQ(result.utpc_center_rounded(), 17);
}

TEST_F(uTPCTest, AnalyzeBadY) {
  hit.weight = 1;
  event.insert(hit);
  auto result = utpcAnalyzer(true, 5, 5).analyze(event);

  EXPECT_FALSE(result.good);
}

TEST_F(uTPCTest, AnalyzeBadX) {
  hit.plane = 1;
  event.insert(hit);
  auto result = utpcAnalyzer(true, 5, 5).analyze(event);
  EXPECT_FALSE(result.good);
}

TEST_F(uTPCTest, AnalyzeGood) {
  hit.weight = 1;
  event.insert(hit);
  hit.plane = 1;
  event.insert(hit);
  auto result = utpcAnalyzer(true, 5, 5).analyze(event);
  EXPECT_TRUE(result.good);
}

TEST_F(uTPCTest, InsertInvalid) {
  hit.weight = 1;
  hit.plane = 0;
  event.insert(hit);
  hit.plane = 1;
  event.insert(hit);
  hit.plane = 2;
  event.insert(hit);
  EXPECT_EQ(2, event.total_hit_count());
}

TEST_F(uTPCTest, DebugPrint) {
  MESSAGE() << "This is not a test, just calling debug print function\n";
  auto result = utpcAnalyzer(true, 5, 5).analyze(event);
  MESSAGE() << result.debug() << "\n";
}

// \todo more & better tests of this required

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
