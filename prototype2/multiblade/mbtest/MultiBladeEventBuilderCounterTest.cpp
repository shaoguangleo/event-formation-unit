/** Copyright (C) 2017-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
///  Unit tests for MultiBladeEventBuilder using Google Test.
/// Here the various counters are tested.
///
/// Author: Carsten Søgaard, Niels Bohr Institute, University of Copenhagen
///
//===----------------------------------------------------------------------===//

/// \todo - nolonger valid - new data format

#include "mbcommon/MultiBladeEventBuilder.h"
#include "MultiBladeTestData.h"
#include "test/TestBase.h"


/// \todo add further checks
TEST(MBEventBuilderTest, Constructor) {
  MultiBladeEventBuilder evbuilder;
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 0);
}

/// not easy to test
TEST(MBEventBuilderTest, AddDataPoint) {
  MultiBladeEventBuilder evbuilder;
  bool retval = evbuilder.addDataPoint(0, 1000, 0); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 1000, 10); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 1000, 10000); // ch, adc, time, outside window
  ASSERT_EQ(retval, true);
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 1);
}

/// not easy to test
TEST(MBEventBuilderTest, AddDataPointBelowADCThreshold) {
  MultiBladeEventBuilder evbuilder;
  evbuilder.setThreshold(1000);
  bool retval = evbuilder.addDataPoint(0, 999, 0); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 999, 10); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 999, 10000); // ch, adc, time, outside window
  ASSERT_EQ(retval, false);
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 0);
}

TEST(MBEventBuilderTest, AddDataPointAboveADCThreshold) {
  MultiBladeEventBuilder evbuilder;
  evbuilder.setThreshold(1000);
  bool retval = evbuilder.addDataPoint(0, 1000, 0); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 1000, 10); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 1000, 10000); // ch, adc, time, outside window
  ASSERT_EQ(retval, true);
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 1);
}

/// not easy to test
TEST(MBEventBuilderTest, AddDataPointInvalidChannels) {
  MultiBladeEventBuilder evbuilder;
  bool retval = evbuilder.addDataPoint(64, 1000, 0); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(64, 1000, 10); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(64, 1000, 10000); // ch, adc, time, outside window
  ASSERT_EQ(retval, false);
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 0);
}

/// not easy to test
TEST(MBEventBuilderTest, AddDataPointValidChannels) {
  MultiBladeEventBuilder evbuilder;
  bool retval = evbuilder.addDataPoint(31, 1000, 0); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(63, 1000, 10); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(63, 1000, 10000); // ch, adc, time, outside window
  ASSERT_EQ(retval, true);
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 1);
}

TEST(MBEventBuilderTest, EventCounter) {

  // Test that events are counted correctly

  // Instanciate the event-builder
  MultiBladeEventBuilder p;

  // Initialize the expected number of events counter
  uint nevents = 1;

  for (uint i = 0; i <= 15; i++) {

    // Read a subset of the test data
    uint begin = i * 5;
    uint end = begin + 5;
    std::vector<uint> datapoint(&data[begin], &data[end]);
    // Wire data-points
    EXPECT_EQ(datapoint[4], p.addDataPoint(datapoint[0], datapoint[2], datapoint[3]));
    // Strip data-points
    EXPECT_FALSE(p.addDataPoint(datapoint[1], datapoint[2], datapoint[3]));

    // Validation of event counter
    if (datapoint[4]) {
      EXPECT_EQ(nevents, p.getNumberOfEvents());
      // Increment validation iterator
      nevents++;
    }
  }

  // Provide a non-adjacent event, which is discarded
  for (uint i = 10; i < 15; i++) {

    // Remove a point (making it non adjacent)
    if (i != 12) {
      // Read a subset of the test data
      uint begin = i * 5;
      uint end = begin + 5;
      std::vector<uint> datapoint(&data[begin], &data[end]);
      // Wire data-points
      EXPECT_EQ(datapoint[4], p.addDataPoint(datapoint[0], datapoint[2], datapoint[3]));
      // Strip data-points
      EXPECT_FALSE(p.addDataPoint(datapoint[1], datapoint[2], datapoint[3]));
    }
  }

  // Validation of event counter at "end of run".
  // The counter was not incremented before, due to the non-adjacent point.
  p.lastPoint();
  EXPECT_EQ(nevents, p.getNumberOfEvents());
}

#if 0
TEST(MBEventBuilder__Test, ClusterCounters) {

  // Test the counters of number of points per event

  // Instaciate the event-counter and configure
  MultiBladeEventBuilder p;
  p.setTimeWindow(config[0]);
  p.setNumberOfWireChannels(config[1]);
  p.setNumberOfStripChannels(config[2]);

  // Validation counters (similar to those in the event-builder).
  // Index 0 = 1 datapoint, index 1 = 2 datapoints, etc.
  std::array<uint64_t, 6> wireclusterpoints = {{0, 0, 0, 0, 0, 0}};
  std::array<uint64_t, 6> stripclusterpoints = {{0, 0, 0, 0, 0, 0}};

  // Counters for number of points per event
  uint wirepoints = 0;
  uint strippoints = 0;

  // Iterator for validation data
  // std::vector<double>::iterator valw = validation_weighted.begin();

  // Test the case when there are both wire and strip points
  for (uint i = 0; i <= 21; i++) {
    uint begin = i * 5;
    uint end = begin + 5;
    std::vector<uint> datapoint(&data[begin], &data[end]);
    // Wire data-points
    bool retval = p.addDataPoint(datapoint[0], datapoint[2], datapoint[3]);
    ASSERT_EQ(retval, datapoint[4]);
    // Strip data-points
    retval = p.addDataPoint(datapoint[1], datapoint[2], datapoint[3]);
    ASSERT_EQ(retval, datapoint[4]);

    if (!datapoint[4]) {

      wirepoints++;
      strippoints++;

    } else {

      // Increment the correct positions in the counters
      wireclusterpoints[wirepoints - 1]++;
      stripclusterpoints[strippoints - 1]++;

      // Check all indexes of the counters for each event.
      EXPECT_EQ(wireclusterpoints[0], p.get2DWireClusterCounter()[0]);
      EXPECT_EQ(wireclusterpoints[1], p.get2DWireClusterCounter()[1]);
      EXPECT_EQ(wireclusterpoints[2], p.get2DWireClusterCounter()[2]);
      EXPECT_EQ(wireclusterpoints[3], p.get2DWireClusterCounter()[3]);
      EXPECT_EQ(wireclusterpoints[4], p.get2DWireClusterCounter()[4]);
      EXPECT_EQ(wireclusterpoints[5], p.get2DWireClusterCounter()[5]);

      EXPECT_EQ(stripclusterpoints[0], p.get2DStripClusterCounter()[0]);
      EXPECT_EQ(stripclusterpoints[1], p.get2DStripClusterCounter()[1]);
      EXPECT_EQ(stripclusterpoints[2], p.get2DStripClusterCounter()[2]);
      EXPECT_EQ(stripclusterpoints[3], p.get2DStripClusterCounter()[3]);
      EXPECT_EQ(stripclusterpoints[4], p.get2DStripClusterCounter()[4]);
      EXPECT_EQ(stripclusterpoints[5], p.get2DStripClusterCounter()[5]);

      // Reset the points counters (1, since a point is provided when finishing
      // the cluster
      wirepoints = 1;
      strippoints = 1;
    }
  }

  // End the "run" and reset all counters
  p.lastPoint();
  p.resetCounters();

  // Reset counters
  wireclusterpoints = {{0, 0, 0, 0, 0, 0}};
  stripclusterpoints = {{0, 0, 0, 0, 0, 0}};

  // Reset the counters to "start of run"
  wirepoints = 0;
  strippoints = 0;

  // Test the case of only wire points
  for (uint i = 0; i <= 21; i++) {
    uint begin = i * 5;
    uint end = begin + 5;
    std::vector<uint> datapoint(&data[begin], &data[end]);
    // Wire data-points
    p.addDataPoint(datapoint[0], datapoint[2], datapoint[3]);

    if (!datapoint[4]) {

      wirepoints++;

    } else {

      // Incement at the correct index.
      wireclusterpoints[wirepoints - 1]++;

      // Test counter at all indexes
      EXPECT_EQ(wireclusterpoints[0], p.get1DWireClusterCounter()[0]);
      EXPECT_EQ(wireclusterpoints[1], p.get1DWireClusterCounter()[1]);
      EXPECT_EQ(wireclusterpoints[2], p.get1DWireClusterCounter()[2]);
      EXPECT_EQ(wireclusterpoints[3], p.get1DWireClusterCounter()[3]);
      EXPECT_EQ(wireclusterpoints[4], p.get1DWireClusterCounter()[4]);
      EXPECT_EQ(wireclusterpoints[5], p.get1DWireClusterCounter()[5]);

      EXPECT_EQ(0, p.get1DStripClusterCounter()[0]);
      EXPECT_EQ(0, p.get1DStripClusterCounter()[1]);
      EXPECT_EQ(0, p.get1DStripClusterCounter()[2]);
      EXPECT_EQ(0, p.get1DStripClusterCounter()[3]);
      EXPECT_EQ(0, p.get1DStripClusterCounter()[4]);
      EXPECT_EQ(0, p.get1DStripClusterCounter()[5]);

      wirepoints = 1;
    }
  }

  // "End of run" and reset all counters
  p.lastPoint();
  p.resetCounters();

  // Reset validation counters
  wireclusterpoints = {{0, 0, 0, 0, 0, 0}};
  stripclusterpoints = {{0, 0, 0, 0, 0, 0}};

  // Reset event counters
  wirepoints = 0;
  strippoints = 0;

  // Test the case of only strip points
  for (uint i = 0; i <= 21; i++) {
    uint begin = i * 5;
    uint end = begin + 5;
    std::vector<uint> datapoint(&data[begin], &data[end]);
    // Strip data-points
    p.addDataPoint(datapoint[1], datapoint[2], datapoint[3]);

    if (!datapoint[4]) {

      strippoints++;

      EXPECT_EQ(strippoints, p.getStripClusterSize());

    } else {

      // Incement at the correct index.
      stripclusterpoints[strippoints - 1]++;

      // Test counter at all indexes
      EXPECT_EQ(0, p.get1DWireClusterCounter()[0]);
      EXPECT_EQ(0, p.get1DWireClusterCounter()[1]);
      EXPECT_EQ(0, p.get1DWireClusterCounter()[2]);
      EXPECT_EQ(0, p.get1DWireClusterCounter()[3]);
      EXPECT_EQ(0, p.get1DWireClusterCounter()[4]);
      EXPECT_EQ(0, p.get1DWireClusterCounter()[5]);

      EXPECT_EQ(stripclusterpoints[0], p.get1DStripClusterCounter()[0]);
      EXPECT_EQ(stripclusterpoints[1], p.get1DStripClusterCounter()[1]);
      EXPECT_EQ(stripclusterpoints[2], p.get1DStripClusterCounter()[2]);
      EXPECT_EQ(stripclusterpoints[3], p.get1DStripClusterCounter()[3]);
      EXPECT_EQ(stripclusterpoints[4], p.get1DStripClusterCounter()[4]);
      EXPECT_EQ(stripclusterpoints[5], p.get1DStripClusterCounter()[5]);

      strippoints = 1;
    }
  }
}


TEST(MBEventBuilder__Test, NoDataRecieved) {

  MultiBladeEventBuilder p;

  EXPECT_EQ(0, p.getNumberOfPositionRejected());

  p.lastPoint();

  EXPECT_DOUBLE_EQ(-1, p.getWirePosition());
  EXPECT_DOUBLE_EQ(-1, p.getStripPosition());

  EXPECT_EQ(1, p.getNumberOfPositionRejected());
}
#endif



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
