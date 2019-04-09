/** Copyright (C) 2018 European Spallation Source ERIC */

#include <common/ReadoutSerializer.h>
#include <common/Producer.h>
#include <test/TestBase.h>

class ReadoutSerializerTest : public TestBase {
  virtual void SetUp() {  }

  virtual void TearDown() {  }

protected:
  static const int arraysize = 10000; // max entries
  static const int fboverhead = 86;   // found by experimentation, now with header!
  static const int entrysize = 10;    // three u16 + one u32

  char flatbuffer[1024 * 1024 * 5];


public:
  void copy_buffer(Buffer<uint8_t> b)
  {
    memcpy(flatbuffer, b.address, b.size);
  }

};

TEST_F(ReadoutSerializerTest, Constructor) {
  ReadoutSerializer serializer(arraysize);
  ASSERT_EQ(0, serializer.getNumEntries());
}

TEST_F(ReadoutSerializerTest, ProduceEmpty) {
  ReadoutSerializer serializer(arraysize);
  ASSERT_EQ(0, serializer.getNumEntries());
  auto res = serializer.produce();
  ASSERT_EQ(res, 0);
}

TEST_F(ReadoutSerializerTest, AddEntries) {
  ReadoutSerializer serializer(arraysize);
  ASSERT_EQ(0, serializer.getNumEntries());
  for (int i = 1; i < arraysize; i++) {
    int res = serializer.addEntry(0,0,0,0);
    ASSERT_EQ(i, serializer.getNumEntries());
    ASSERT_EQ(res, 0);
  }
  int res = serializer.addEntry(0,0,0,0);
  ASSERT_EQ(0, serializer.getNumEntries());
  ASSERT_EQ(res != 0, true);
}

TEST_F(ReadoutSerializerTest, ManualProduce) {
  for (int maxlen = 10; maxlen < 1000; maxlen++) {
    ReadoutSerializer serializer(maxlen);
    serializer.set_callback(std::bind(&ReadoutSerializerTest::copy_buffer,
        this, std::placeholders::_1));

    ASSERT_EQ(0, serializer.getNumEntries());
    int res = serializer.addEntry(0,0,0,0);
    ASSERT_EQ(res, 0);
    res = serializer.produce();
    ASSERT_GT(res, 0);
    ASSERT_LE(res, maxlen*entrysize + fboverhead);

    EXPECT_EQ(std::string(&flatbuffer[4], 4), "mo01");
  }
}

TEST_F(ReadoutSerializerTest, CheckSmallSizes) {
  for (int maxlen = 10; maxlen < 1000; maxlen++) {
    ReadoutSerializer serializer(maxlen);
    for (int i = 1; i < maxlen; i++) {
      int res = serializer.addEntry(0,0,0,0);
      ASSERT_EQ(res, 0);
    }
    int res = serializer.addEntry(0,0,0,0);
    ASSERT_GT(res, 0);
    ASSERT_LE(res, maxlen*entrysize + fboverhead);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
