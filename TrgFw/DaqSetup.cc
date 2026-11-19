#include "DaqSetup.hh"

#include "AdcFuncs.hh"
#include "DaqOptions.hh"
#include "FPGAModule.hh"
#include "MhTdcFuncs.hh"
#include "RegisterMap.hh"
#include "SelfTriggerConfig.hh"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace HUL::DAQ
{
    namespace
    {
        void ConfigureHitMask(HUL::FPGAModule &fpga_module, std::uint32_t mask)
        {
            const std::uint32_t addresses[] = {LBUS::TDC::kAddrHitMask, LBUS::ADC::kAddrHitMask};
            for (const auto base : addresses)
            {
                for (std::uint32_t byte = 0; byte < 4; ++byte)
                {
                    fpga_module.WriteModule(base + (byte << 16), (mask >> (byte * 8)) & 0xff, 1);
                }
            }
            for (const auto base : addresses)
            {
                for (std::uint32_t byte = 0; byte < 4; ++byte)
                {
                    const auto address = base + (byte << 16);
                    const auto expected = (mask >> (byte * 8)) & 0xff;
                    if (fpga_module.ReadModule(address, 1) != expected)
                    {
                        throw std::runtime_error("Hit mask register read-back failed at address " + std::to_string(address));
                    }
                }
            }
            std::cout << "#D: TDC/ADC hit mask register read-back: OK\n";
        }

        void ConfigureSelfTrigger(HUL::FPGAModule &fpga_module, const SelfTriggerConfig &config)
        {
            SetTdcSelfTrigger(config, fpga_module);

            if (VerifyTdcSelfTrigger(config, fpga_module))
            {
                std::cout << "#D: Self Trigger register read-back: OK\n";
                return;
            }

            // Keep the self-trigger disabled after an incomplete update.
            fpga_module.WriteModule(LBUS::TDC::kAddrSelfHitThreshold, 0);
            throw std::runtime_error("Self Trigger register read-back failed");
        }

        void ConfigureTdc(HUL::FPGAModule &fpga_module, const DaqOptions &options)
        {
            SetTdcWindow(options.window_max, options.window_min, fpga_module);

            const std::uint32_t en_block = LBUS::TDC::kEnLeading | LBUS::TDC::kEnTrailing;
            fpga_module.WriteModule(LBUS::TDC::kAddrEnableBlock, en_block);
        }

        void ConfigureAdc(HUL::FPGAModule &fpga_module, const DaqOptions &options)
        {
            SetAdcWindow(options.window_max, options.window_min, options.enable_zero_suppression, options.hit_timeout, fpga_module);
        }
    }

    void ConfigureHardware(HUL::FPGAModule &fpga_module, const DaqOptions &options, const SelfTriggerConfig &self_trigger_config, std::uint32_t hit_mask)
    {
        ConfigureHitMask(fpga_module, hit_mask);
        ConfigureSelfTrigger(fpga_module, self_trigger_config);

        // Release AdcRo reset
        if (fpga_module.ReadModule(LBUS::ADC::kAddrAdcRoReset) == 1)
        {
            fpga_module.WriteModule(LBUS::ADC::kAddrAdcRoReset, 0);
        }

        // Set trigger path //
        const std::uint32_t reg_trg = LBUS::TRM::kRegL1Ext;
        fpga_module.WriteModule(LBUS::TRM::kAddrSelectTrigger, reg_trg);

        // Set NIM-IN //
        fpga_module.WriteModule(LBUS::IOM::kAddrExtL1, LBUS::IOM::kReg_i_Nimin1);

        ConfigureTdc(fpga_module, options);

        ConfigureAdc(fpga_module, options);

        // Reset event counter //
        fpga_module.WriteModule(LBUS::DCT::kAddrResetEvb, 0);

        // AdcRo initialize status
        std::cout << "#D: AdcRo IsReady status: " << std::hex << fpga_module.ReadModule(LBUS::ADC::kAddrAdcRoIsReady) << std::dec << std::endl;
    }
}
