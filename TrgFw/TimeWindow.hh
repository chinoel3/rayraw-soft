#ifndef RAYRAW_TIME_WINDOW_HH
#define RAYRAW_TIME_WINDOW_HH

#include <cstdint>
#include <stdexcept>

namespace HUL::DAQ {

inline void ValidateTimeWindow(std::uint32_t window_max, std::uint32_t window_min)
{
  constexpr std::uint32_t maximum = 2047;
  if (window_max > maximum || window_min > maximum) {
    throw std::runtime_error("Time window values must be in the range 0..2047 (13.333... ns step)");
  }
  if (window_min > window_max) {
    throw std::runtime_error("Time window min must not be larger than time window max");
  }
}

}
#endif
