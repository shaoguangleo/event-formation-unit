/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <multiblade/clustering/EventBuilder.h>
#include <algorithm>
#include <fmt/format.h>

#include <common/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

void EventBuilder::insert(Hit hit) {
  if (hit.plane == 0) {
    p0.push_back(hit);
  }
  else if (hit.plane == 1) {
    p1.push_back(hit);
  }
  else {
    XTRACE(CLUSTER, WAR, "bad plane %s", hit.debug().c_str());
  }
}

void EventBuilder::flush() {
  matcher.matched_events.clear();

  std::sort(p0.begin(), p0.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.time < e2.time;
            });
  c0.cluster(p0);
  c0.flush();

  std::sort(p1.begin(), p1.end(),
            [](const Hit &e1, const Hit &e2) {
              return e1.time < e2.time;
            });
  c1.cluster(p1);
  c1.flush();

  matcher.insert(0, c0.clusters);
  matcher.insert(1, c1.clusters);
  matcher.match(true);

  clear();
}

void EventBuilder::clear() {
  p0.clear();
  p1.clear();
}


}
