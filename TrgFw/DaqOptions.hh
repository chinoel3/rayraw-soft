#ifndef RAYRAW_DAQ_OPTIONS_HH
#define RAYRAW_DAQ_OPTIONS_HH

#include <cstdint>
#include <exception>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include "TimeWindow.hh"

namespace HUL::DAQ {

struct DaqOptions {
  std::string board_ip;
  std::int32_t run_no;
  std::int32_t num_event;
  std::uint32_t window_max;
  std::uint32_t window_min;
};

inline void PrintDaqUsage(std::ostream& out, const char* program)
{
  out << "Usage\n"
      << program << " [IP address] [Run No] [Num of events] [Tdc window max] [Tdc window min]\n"
      << " - Run No         : Run number\n"
      << " - Num of events  : Max event number. DAQ stops at this count or by Ctrl-C.\n"
      << " - Tdc window max : Max TDC window value (0..2047, 13.333... ns step).\n"
      << " - Tdc window min : Min TDC window value (0..max, 13.333... ns step).\n"
      << " - Example        : " << program << " 192.168.20.101 1 1000 150 50\n"
      << " - Help           : --help or -h\n";
}

inline std::int32_t ParseDaqInteger(const char* text, const char* name)
{
  std::size_t parsed = 0;
  long long value = 0;
  try {
    value = std::stoll(text, &parsed, 10);
  } catch (const std::exception&) {
    throw std::runtime_error("Invalid " + std::string(name) + ": " + text + " (expected a 32-bit decimal integer)");
  }
  if (parsed != std::string(text).size()
      || value < std::numeric_limits<std::int32_t>::min()
      || value > std::numeric_limits<std::int32_t>::max()) {
    throw std::runtime_error("Invalid " + std::string(name) + ": " + text + " (expected a 32-bit decimal integer)");
  }
  return static_cast<std::int32_t>(value);
}

inline DaqOptions ParseDaqOptions(int argc, char* argv[])
{
  if (argc != 6) {
    throw std::runtime_error("Expected exactly 5 arguments: IP address, run number, event count, window max, window min");
  }
  DaqOptions options{};
  options.board_ip = argv[1];
  options.run_no = ParseDaqInteger(argv[2], "run number");
  options.num_event = ParseDaqInteger(argv[3], "event count");
  const auto window_max = ParseDaqInteger(argv[4], "time window max");
  const auto window_min = ParseDaqInteger(argv[5], "time window min");
  if (window_max < 0 || window_min < 0) {
    throw std::runtime_error("Time window values must be in the range 0..2047 (13.333... ns step)");
  }
  options.window_max = static_cast<std::uint32_t>(window_max);
  options.window_min = static_cast<std::uint32_t>(window_min);
  ValidateTimeWindow(options.window_max, options.window_min);
  return options;
}

}
#endif
