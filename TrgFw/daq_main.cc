#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "AdcFuncs.hh"
#include "DaqFuncs.hh"
#include "FPGAModule.hh"
#include "MhTdcFuncs.hh"
#include "RegisterMap.hh"
#include "UDPRBCP.hh"

namespace
{
	enum kArgIndex
	{
		kBin,
		kIp,
		kRunNo,
		kNumEvent,
		kWinMax,
		kWinMin,
		kFirstOption
	};

	void PrintUsage(const char *program)
	{
		std::cout << "Usage\n";
		std::cout << program << " [IP address] [Run No] [Num of events] [Tdc window max] [Tdc window min] [options]\n";
		std::cout << "\t- Run No            : Run number\n";
		std::cout << "\t- Num of events     : Max event number. DAQ will stop at this event number or by Ctrl-C.\n";
		std::cout << "\t- Tdc window max    : Max TDC window value. (10 ns step)\n";
		std::cout << "\t- Tdc window min    : Min TDC window value. (10 ns step)\n\n";

		std::cout << "Options\n";
		std::cout << "Enable Zero Suppression data acquisition with the following options:\n";
		std::cout << "\t--enable-zerosup\n";
		std::cout << "\t--enable_zerosup\n\n";
	}

	int32_t ParseInt32(const char *text, const char *name)
	{
		std::size_t parsed_length = 0;
		const std::string value_text(text);
		const long long value = std::stoll(value_text, &parsed_length, 0);

		if (parsed_length != value_text.size() || value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max())
		{
			throw std::runtime_error(std::string("Invalid ") + name + ": " + value_text);
		}

		return static_cast<int32_t>(value);
	}
}

using namespace LBUS;

int main(int argc, char *argv[])
{
	if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h"))
	{
		PrintUsage(argv[kBin]);
		return 0;
	}

	if (argc < kFirstOption)
	{
		PrintUsage(argv[kBin]);
		return argc == 1 ? 0 : 1;
	}

	try
	{
		const std::string board_ip = argv[kIp];
		const int32_t run_no       = ParseInt32(argv[kRunNo], "run number");
		const int32_t num_event    = ParseInt32(argv[kNumEvent], "number of events");
		const int32_t window_max   = ParseInt32(argv[kWinMax], "TDC window max");
		const int32_t window_min   = ParseInt32(argv[kWinMin], "TDC window min");

		bool enable_zerosup = false;

		for (int i = kFirstOption; i < argc; ++i)
		{
			const std::string argument = argv[i];

			if (argument == "--enable-zerosup" || argument == "--enable_zerosup")
			{
				enable_zerosup = true;
				continue;
			}

			throw std::runtime_error("Unknown option: " + argument);
		}

		if (window_min < 0 || window_max < 0 || window_max > 2047)
		{
			throw std::runtime_error("TDC window values must be in the range 0 to 2047");
		}

		if (window_min > window_max)
		{
			throw std::runtime_error("Min TDC window value must not be larger than Max TDC window value");
		}

		const HUL::DAQ::SelfTriggerConfig self_config = HUL::DAQ::LoadSelfTriggerConfig();

		RBCP::UDPRBCP udp_rbcp(board_ip, RBCP::gUdpPort, RBCP::DebugMode::kNoDisp);
		HUL::FPGAModule fpga_module(udp_rbcp);

		HUL::DAQ::SetTdcSelfTrigger(self_config, fpga_module);

		if (!HUL::DAQ::VerifyTdcSelfTrigger(self_config, fpga_module))
		{
			// Leave self-trigger disabled if register programming was incomplete.
			fpga_module.WriteModule(TDC::kAddrSelfHitThreshold, 0);
			throw std::runtime_error("Self Trigger register read-back failed");
		}

		std::cout << "#D: Self Trigger register read-back: OK\n";

		// Release AdcRo reset
		if (fpga_module.ReadModule(ADC::kAddrAdcRoReset) == 1)
		{
			fpga_module.WriteModule(ADC::kAddrAdcRoReset, 0);
		}

		// SelfL1 is not used; keep the DAQ trigger path on external L1 only.
		const uint32_t reg_trg = TRM::kRegL1Ext;
		fpga_module.WriteModule(TRM::kAddrSelectTrigger, reg_trg);

		// Set NIM-IN //
		fpga_module.WriteModule(IOM::kAddrExtL1, IOM::kReg_i_Nimin1);

		// Set TDC window //
		HUL::DAQ::SetTdcWindow(window_max, window_min, fpga_module);

		// Enable TDC block //
		const uint32_t en_block = TDC::kEnLeading | TDC::kEnTrailing;
		fpga_module.WriteModule(TDC::kAddrEnableBlock, en_block);

		// Set ADC window //
		HUL::DAQ::SetAdcWindow(window_max, window_min, enable_zerosup ? 1U : 0U, fpga_module);

		// Reset event counter //
		fpga_module.WriteModule(DCT::kAddrResetEvb, 0);

		// AdcRo initialize status
		std::cout << "#D: AdcRo IsReady status: " << std::hex << fpga_module.ReadModule(ADC::kAddrAdcRoIsReady) << std::dec << std::endl;

		// Event Read Cycle //
		HUL::DAQ::DoTrgDaq(board_ip, run_no, num_event, DCT::kAddrDaqGate);
	}
	catch (const std::exception &error)
	{
		std::cerr << "#E: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}

