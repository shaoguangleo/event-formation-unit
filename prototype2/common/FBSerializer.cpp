/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/FBSerializer.h>
#include <libs/include/gccintel.h>

#define TIMESIZE 4
#define PIXELSIZE 4

FBSerializer::FBSerializer(size_t maxarraylength) : builder(maxarraylength * 8 + 2048)
     , maxlen(maxarraylength) {
}

FBSerializer::~FBSerializer() {
}

int FBSerializer::serialize(uint64_t time, uint64_t seqno, char * timearr, char * pixarr,
                            size_t entries, unsigned char ** buffer, size_t * reslen) {
  assert(entries <= maxlen);

  builder.Clear();

  auto timeoff = builder.CreateUninitializedVector(entries * TIMESIZE, 1, &timeptr);
  std::memcpy(timeptr, timearr, entries * TIMESIZE);

  auto pixeloff = builder.CreateUninitializedVector(entries * PIXELSIZE, 1, &pixelptr);
  std::memcpy(pixelptr, pixarr, entries * PIXELSIZE);

  auto msg = CreateEventMessage(builder, 0, seqno, time, timeoff, pixeloff);

  builder.Finish(msg);
  *buffer = builder.GetBufferPointer();
  *reslen = builder.GetSize();
  return *reslen;
}