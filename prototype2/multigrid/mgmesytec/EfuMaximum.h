/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <multigrid/mgmesytec/Efu.h>
#include <limits>

namespace Multigrid {

class EfuMaximum : public Efu {
public:
  EfuMaximum() = default;
  ~EfuMaximum() = default;

  size_t ingest(const std::vector<Hit>& hits) override;

  void reset();
  bool ingest(const Hit& hit);

  bool event_good() const override;

  uint32_t x() const override;
  uint32_t y() const override;
  uint32_t z() const override;
  uint64_t time() const override;

private:
  uint16_t GridAdcMax {0};
  uint16_t WireAdcMax {0};

  bool WireGood{false};
  bool GridGood{false};

  uint32_t x_;
  uint32_t y_;
  uint32_t z_;

  uint64_t time_ {0};
};

}