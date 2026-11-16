#ifndef HIT_MASK_CONFIG_HH
#define HIT_MASK_CONFIG_HH

#include <cstdint>
#include <iosfwd>
#include <string>

namespace HUL::DAQ
{
    std::uint32_t ParseHitMaskConfig(std::istream &input);
    std::uint32_t LoadHitMaskConfig(const std::string &filename);
    std::uint32_t LoadHitMaskConfig();
}

#endif

