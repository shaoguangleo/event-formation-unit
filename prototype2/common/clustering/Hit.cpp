/** Copyright (C) 2016, 2017 European Spallation Source ERIC */
//===----------------------------------------------------------------------===//
///
/// \file Hit.cpp
/// \brief Hit implementation
///
//===----------------------------------------------------------------------===//

#include <common/clustering/Hit.h>
#include <fmt/format.h>

std::string Hit::debug() const {
  return fmt::format("time={} plane={} coord={} weight={}",
                     time, plane, coordinate, weight);
}
