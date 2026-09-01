#ifndef ADC_FUNCS_HH
#define ADC_FUNCS_HH

#include <stdint.h>

namespace HUL
{
	class FPGAModule;
}

namespace HUL::DAQ
{
	void SetAdcWindow(uint32_t wmax, uint32_t wmin, bool en_zerosup, uint8_t hit_timeout, HUL::FPGAModule &fpga_module);
}

#endif
