/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Event.h>
#include <cmath>
#include <sstream>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Gem {

void Event::insert_hit(const Hit &e) {
  if (e.plane == 1) { /**< \todo deal with multiple panels */
    y.insert(e);
  } else if (e.plane == 0) {
    x.insert(e);
  }
}

void Event::merge(UtpcCluster &cluster) {
  if (cluster.plane() == 1) { /**< \todo deal with multiple panels */
    y.merge(cluster);
  } else if (cluster.plane() == 0) {
    x.merge(cluster);
  }
}

bool Event::empty() const
{
  return x.empty() && y.empty();
}

uint64_t Event::time_end() const
{
  if (x.empty())
    return y.time_end();
  if (y.empty())
    return x.time_end();
  return std::max(x.time_end(), y.time_end());
}

uint64_t Event::time_start() const
{
  if (x.empty())
    return y.time_start();
  if (y.empty())
    return x.time_start();
  return std::min(x.time_start(), y.time_start());
}

uint64_t Event::time_span() const {
  return (time_end() - time_start());
}

uint64_t Event::time_overlap(const UtpcCluster &other) const {
  auto latest_start = std::max(other.time_start(), time_start());
  auto earliest_end = std::min(other.time_end(), time_end());
  if (latest_start > earliest_end)
    return 0;
  return (earliest_end - latest_start);
}

bool Event::time_overlap_thresh(const UtpcCluster &other, uint64_t thresh) const {
  auto ovr = time_overlap(other);
  return (((ovr / other.time_span()) + (ovr / time_span())) > thresh);
}

void Event::analyze(bool weighted, int16_t max_timebins,
                    int16_t max_timedif) {
  XTRACE(PROCESS, DEB, "x.entries.size(): %lu, y.entries.size(): %lu",
         x.hit_count(), y.hit_count());
  if (x.hit_count()) {
    x.analyze(weighted, max_timebins, max_timedif);
  }
  if (y.hit_count()) {
    y.analyze(weighted, max_timebins, max_timedif);
  }
  valid_ = x.hit_count() && y.hit_count();
  if (valid_) {
    utpc_time_ = std::max(x.time_end(), y.time_end());
  }
}

bool Event::valid() const {
  return valid_;
}

bool Event::meets_lower_criterion(int16_t max_lu) const {
  return (x.uncert_lower < max_lu) && (y.uncert_lower < max_lu);
}

uint64_t Event::utpc_time() const {
  return utpc_time_;
}

std::string Event::debug() const {
  std::stringstream ss;
  ss << "Tstart=" << utpc_time_;
  if (valid_)
    ss << "  GOOD\n";
  else
    ss << "  BAD\n";
  ss << "  X:\n" << x.debug();
  ss << "  Y:\n" << y.debug();
  return ss.str();
}

void Event::debug2() {
  printf("x strips: %s", x.debug(true).c_str());
  printf("y strips: %s", y.debug(true).c_str());
}

}
