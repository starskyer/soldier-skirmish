#include "frameClock.h"
#include <algorithm>
#include <cmath>

namespace skirmish {
int FixedStepClock::consume(double elapsed, bool active) {
    if (!active)
        return 0;
    if (!std::isfinite(elapsed) || elapsed < 0)
        throw std::invalid_argument("Invalid elapsed time");
    constexpr double step = 1.0 / TicksPerSecond;
    accumulator_ += std::min(elapsed, 0.25);
    const int count = std::min(8, static_cast<int>((accumulator_ + 1e-12) / step));
    accumulator_ = std::max(0.0, accumulator_ - count * step);
    if (accumulator_ >= step)
        accumulator_ = std::fmod(accumulator_, step);
    return count;
}
} // namespace skirmish
