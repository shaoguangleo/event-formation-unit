/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Classes for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/nmx/Cluster.h>
#include <limits>
#include <list>
#include <string>

namespace Gem {

class Event {
public:
  Cluster x, y; /// tracks in x and y planes

  /// \brief adds hit to event
  /// \param hit to be added
  void insert_hit(const Hit &e);

  void merge(Cluster &cluster);

  bool empty() const;

  uint64_t time_end() const;
  uint64_t time_start() const;
  uint64_t time_span() const;
  uint64_t time_overlap(const Cluster &other) const;
  bool time_overlap_thresh(const Cluster &other, uint64_t thresh) const;

  /// \brief analyzes particle track
  /// \param weighted determine entry strip using weighted average
  /// \param max_timebins maximum number of timebins to consider for upper
  /// uncertainty
  /// \param max_timedif maximum span of timebins to consider for upper
  /// uncertainty
  void analyze(bool weighted, int16_t max_timebins, int16_t max_timedif);

  /// \brief indicates if entry strips were determined in for both planes
  bool valid() const;

  /// \brief returns timestamp for start of event (earlier of 2 planes)
  uint64_t utpc_time() const;

  /// \brief indicates if both dimensions meet lower uncertainty criterion
  bool meets_lower_criterion(int16_t max_lu) const;

  /// \brief prints values for debug purposes
  std::string debug() const;
  void debug2();

private:
  bool valid_{false};   /// event has valid entry strips in both planes
  uint64_t utpc_time_{0}; /// start of event timestamp (earlier of 2 planes)
};

}
