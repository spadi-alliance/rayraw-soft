#include "AdcFuncs.hh"

#include "FPGAModule.hh"
#include "RegisterMap.hh"

namespace HUL::DAQ
{
	namespace
	{
		const uint32_t kCMax      = 2047;
		const uint32_t kPtrDiffWr = 2;
	}

	void SetAdcWindow(uint32_t wmax, uint32_t wmin, bool en_zerosup, uint8_t hit_timeout, HUL::FPGAModule& fpga_module)
	{
		const uint32_t ptr_ofs = kCMax - wmax + kPtrDiffWr;

		fpga_module.WriteModule(LBUS::ADC::kAddrPtrOfs, ptr_ofs, 2);
		fpga_module.WriteModule(LBUS::ADC::kAddrWindowMax, wmax, 2);
		fpga_module.WriteModule(LBUS::ADC::kAddrWindowMin, wmin, 2);
		fpga_module.WriteModule(LBUS::ADC::kAddrEnableZeroSup, en_zerosup, 1);
		fpga_module.WriteModule(LBUS::ADC::kAddrHitTimeout, hit_timeout, 1);
	}
}
