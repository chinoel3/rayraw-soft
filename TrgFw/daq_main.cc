#include <iostream>
#include <unistd.h>

#include "RegisterMap.hh"
#include "FPGAModule.hh"
#include "UDPRBCP.hh"
#include "MhTdcFuncs.hh"
#include "AdcFuncs.hh"
#include "DaqFuncs.hh"
#include "DaqOptions.hh"

using namespace LBUS;

int main(int argc, char* argv[])
{
  if (argc == 1 || (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h"))) {
    HUL::DAQ::PrintDaqUsage(std::cout, argv[0]);
    return 0;
  }

  HUL::DAQ::DaqOptions options{};
  try {
    options = HUL::DAQ::ParseDaqOptions(argc, argv);
  } catch (const std::exception& error) {
    std::cerr << "#E: " << error.what() << '\n';
    HUL::DAQ::PrintDaqUsage(std::cerr, argv[0]);
    return 1;
  }
  
  // body ------------------------------------------------------
  std::string board_ip   = options.board_ip;
  int32_t     run_no     = options.run_no;
  int32_t     num_event  = options.num_event;
  uint32_t    window_max = options.window_max;
  uint32_t    window_min = options.window_min;

  RBCP::UDPRBCP udp_rbcp(board_ip, RBCP::gUdpPort, RBCP::DebugMode::kNoDisp);
  HUL::FPGAModule fpga_module(udp_rbcp);

  // Release AdcRo reset
  if(1 == fpga_module.ReadModule(ADC::kAddrAdcRoReset)){
    fpga_module.WriteModule(ADC::kAddrAdcRoReset, 0);
  }

  // Set trigger path //
  uint32_t reg_trg = TRM::kRegL1Ext;
  fpga_module.WriteModule(TRM::kAddrSelectTrigger, reg_trg);

  // Set NIM-IN //
  fpga_module.WriteModule(IOM::kAddrExtL1, IOM::kReg_i_Nimin1);

  // Set TDC window //
  HUL::DAQ::SetTdcWindow(window_max, window_min, fpga_module);

  // Enable TDC block //
  uint32_t en_block = TDC::kEnLeading | TDC::kEnTrailing;
  fpga_module.WriteModule(TDC::kAddrEnableBlock, en_block);

  // Set ADC window (same as TDC) //
  HUL::DAQ::SetAdcWindow(window_max, window_min, fpga_module);

  // Resest event counter //
  fpga_module.WriteModule(DCT::kAddrResetEvb, 0);

  // AdcRo initialize status
  std::cout << "#D: AdcRo IsReady status: " << std::hex << fpga_module.ReadModule(ADC::kAddrAdcRoIsReady) << std::dec << std::endl;

  // Event Read Cycle //
  HUL::DAQ::DoTrgDaq(board_ip, run_no, num_event, DCT::kAddrDaqGate);
  
  return 0;

}// main
