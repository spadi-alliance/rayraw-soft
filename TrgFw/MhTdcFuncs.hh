#ifndef MHTDC_FUNCS_HH
#define MHTDC_FUNCS_HH

#include <cstdint>
#include <vector>

#include "FPGAModule.hh"
#include "RegisterMap.hh"

namespace HUL::DAQ
{
	struct SelfTriggerConfig
	{
		uint32_t hit_threshold{0};
		uint32_t latch_window{0};
		std::vector<uint32_t> geometry_masks;
	};

	SelfTriggerConfig LoadSelfTriggerConfig();

	inline void
	SetTdcSelfTrigger(const SelfTriggerConfig &config, HUL::FPGAModule &fpga_module)
	{
		using namespace LBUS;

		// Disable the trigger while its related registers are being updated.
		fpga_module.WriteModule(TDC::kAddrSelfHitThreshold, 0);
		fpga_module.WriteModule(TDC::kAddrSelfLatchWindow, config.latch_window);

		for (uint32_t i = 0; i < TDC::kNumSelfGeometryMax; ++i)
		{
			const uint32_t mask = i < config.geometry_masks.size() ? config.geometry_masks[i] : 0;

			fpga_module.WriteModule(TDC::kAddrSelfGeometryBase + i * TDC::kSelfGeometryStride, mask, 4);
		}

		fpga_module.WriteModule(TDC::kAddrSelfGeometryCount, static_cast<uint32_t>(config.geometry_masks.size()));

		// Writing the threshold last enables the trigger only after setup completes.
		fpga_module.WriteModule(TDC::kAddrSelfHitThreshold, config.hit_threshold);
	}

	inline bool
	VerifyTdcSelfTrigger(const SelfTriggerConfig &config, HUL::FPGAModule &fpga_module)
	{
		using namespace LBUS;

		if (fpga_module.ReadModule(TDC::kAddrSelfHitThreshold) != config.hit_threshold)
		{
			return false;
		}

		if (fpga_module.ReadModule(TDC::kAddrSelfLatchWindow) != config.latch_window)
		{
			return false;
		}

		if (fpga_module.ReadModule(TDC::kAddrSelfGeometryCount) != config.geometry_masks.size())
		{
			return false;
		}

		for (uint32_t i = 0; i < TDC::kNumSelfGeometryMax; ++i)
		{
			const uint32_t expected = i < config.geometry_masks.size() ? config.geometry_masks[i] : 0;
			const uint32_t actual = fpga_module.ReadModule(TDC::kAddrSelfGeometryBase + i * TDC::kSelfGeometryStride, 4);

			if (actual != expected)
			{
				return false;
			}
		}

		return true;
	}

	inline void
	SetTdcWindow(uint32_t wmax, uint32_t wmin, HUL::FPGAModule &fpga_module)
	{
		using namespace LBUS;

		static const uint32_t kCMax = 2047;
		static const uint32_t kPtrDiffWr = 2;

		const uint32_t ptr_ofs = kCMax - wmax + kPtrDiffWr;

		fpga_module.WriteModule(TDC::kAddrPtrOfs, ptr_ofs, 2);
		fpga_module.WriteModule(TDC::kAddrWindowMax, wmax, 2);
		fpga_module.WriteModule(TDC::kAddrWindowMin, wmin, 2);
	}
}

#endif

