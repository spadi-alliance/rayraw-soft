#ifndef MHTDC_FUNCS_HH
#define MHTDC_FUNCS_HH

#include <cstdint>

namespace HUL
{
	class FPGAModule;
}

namespace HUL::DAQ
{
	struct SelfTriggerConfig;

	void SetTdcSelfTrigger(const SelfTriggerConfig &config, HUL::FPGAModule &fpga_module);

	bool VerifyTdcSelfTrigger(const SelfTriggerConfig &config, HUL::FPGAModule &fpga_module);

	void SetTdcWindow(std::uint32_t wmax, std::uint32_t wmin, HUL::FPGAModule &fpga_module);
}

#endif
