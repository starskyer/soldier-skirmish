#pragma once
#include "simulationTypes.h"

namespace skirmish {
class FixedStepClock {
  public:
    int consume(double elapsed, bool active);
    double alpha() const { return accumulator_ * TicksPerSecond; }
    void reset() { accumulator_ = 0; }

  private:
    double accumulator_ = 0;
};
} // namespace skirmish
