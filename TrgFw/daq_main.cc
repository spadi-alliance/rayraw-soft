#include <exception>
#include <iostream>

#include "DaqFuncs.hh"
#include "DaqOptions.hh"
#include "DaqSetup.hh"
#include "FPGAModule.hh"
#include "RegisterMap.hh"
#include "SelfTriggerConfig.hh"
#include "UDPRBCP.hh"

namespace
{
	int Run(const HUL::DAQ::DaqOptions &options)
	{
		const HUL::DAQ::SelfTriggerConfig self_trigger_config = HUL::DAQ::LoadSelfTriggerConfig();

		RBCP::UDPRBCP udp_rbcp(options.board_ip, RBCP::gUdpPort, RBCP::DebugMode::kNoDisp);
		HUL::FPGAModule fpga_module(udp_rbcp);

		HUL::DAQ::ConfigureHardware(fpga_module, options, self_trigger_config);

		HUL::DAQ::DoTrgDaq(options.board_ip, options.run_no, options.num_event, LBUS::DCT::kAddrDaqGate);

		return 0;
	}
}

int main(int argc, char *argv[])
{
	if (HUL::DAQ::IsHelpRequested(argc, argv))
	{
		HUL::DAQ::PrintUsage(std::cout, argv[0]);
		return 0;
	}

	if (!HUL::DAQ::HasRequiredArguments(argc))
	{
		HUL::DAQ::PrintUsage(argc == 1 ? std::cout : std::cerr, argv[0]);
		return argc == 1 ? 0 : 1;
	}

	try
	{
		return Run(HUL::DAQ::ParseDaqOptions(argc, argv));
	}
	catch (const std::exception &error)
	{
		std::cerr << "#E: " << error.what() << '\n';
		return 1;
	}
}
