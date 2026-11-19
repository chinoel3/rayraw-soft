#include "HitMaskConfig.hh"

#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifndef RAYRAW_CONFIG_DIR
#define RAYRAW_CONFIG_DIR "."
#endif

namespace HUL::DAQ
{
    std::uint32_t ParseHitMaskConfig(std::istream &input)
    {
        std::array<bool, 32> seen{};
        std::uint32_t mask = 0;
        std::size_t line_number = 0;
        std::string line;

        while (std::getline(input, line))
        {
            ++line_number;
            const auto comment = line.find('#');
            if (comment != std::string::npos)
            {
                line.erase(comment);
            }

            std::istringstream row(line);
            row >> std::ws;
            if (row.eof())
            {
                continue;
            }

            int channel = 0;
            int masked = 0;
            std::string extra;
            if (!(row >> channel >> masked) || (row >> extra) || channel < 0 || channel >= 32 || (masked != 0 && masked != 1))
            {
                throw std::runtime_error("Invalid hit mask at line " + std::to_string(line_number) + ": expected <channel 0..31> <mask 0 or 1>");
            }
            if (seen[channel])
            {
                throw std::runtime_error("Duplicate hit mask channel at line " + std::to_string(line_number));
            }
            seen[channel] = true;
            if (masked == 1)
            {
                mask |= std::uint32_t{1} << channel;
            }
        }

        if (input.bad())
        {
            throw std::runtime_error("Failed while reading hit mask config");
        }
        for (std::size_t channel = 0; channel < seen.size(); ++channel)
        {
            if (!seen[channel])
            {
                throw std::runtime_error("Missing hit mask channel: " + std::to_string(channel));
            }
        }
        return mask;
    }

    std::uint32_t LoadHitMaskConfig(const std::string &filename)
    {
        std::ifstream input(filename);
        if (!input.is_open())
        {
            throw std::runtime_error("Cannot open hit mask config: " + filename);
        }
        const auto mask = ParseHitMaskConfig(input);
        std::cout << "#D: Hit mask config: " << filename << '\n';
        std::cout << "#D: TDC/ADC hit mask: 0x" << std::hex << mask << std::dec << '\n';
        return mask;
    }

    std::uint32_t LoadHitMaskConfig()
    {
        return LoadHitMaskConfig(std::string(RAYRAW_CONFIG_DIR) + "/hit_mask.txt");
    }
}

