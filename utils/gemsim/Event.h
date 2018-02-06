/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Base class for Events
 */

#pragma once

#include <string>

class Simulator;

class Event {
public:
  double time;

  struct eventComparator {
      bool operator() (const Event * left, const Event * right) const {
          return left->time > right->time;
      }
  };

  Event(double event_time) : time(event_time) { }

  virtual void execute(Simulator * sim) {
    printf("Generic event - time %f\n", time);
  }
};