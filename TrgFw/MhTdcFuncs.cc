#include "MhTdcFuncs.hh"

#include "FPGAModule.hh"
#include "RegisterMap.hh"
#include "SelfTriggerConfig.hh"

#include <cstddef>

namespace HUL::DAQ
{
	namespace
	{
		const std::uint32_t kCMax      = 2047;
		const std::uint32_t kPtrDiffWr = 2;

		std::uint32_t GeometryMaskAt(const SelfTriggerConfig &config, std::uint32_t index)
		{
			if (index >= config.geometry_masks.size())
			{
				return 0;
			}

			return config.geometry_masks[index];
		}
	}

	void SetTdcSelfTrigger(const SelfTriggerConfig &config, HUL::FPGAModule &fpga_module)
	{
		// Disable the trigger while its related registers are being updated.
		fpga_module.WriteModule(LBUS::TDC::kAddrSelfHitThreshold, 0);
		fpga_module.WriteModule(LBUS::TDC::kAddrSelfLatchWindow, config.latch_window);

		for (std::uint32_t index = 0; index < LBUS::TDC::kMaxSelfGeometry; ++index)
		{
			fpga_module.WriteModule(LBUS::TDC::kAddrSelfGeometryBase + index * LBUS::TDC::kSelfGeometryStride, GeometryMaskAt(config, index), 4);
		}

		fpga_module.WriteModule(LBUS::TDC::kAddrSelfGeometryCount, static_cast<std::uint32_t>(config.geometry_masks.size()));

		// Enable the trigger only after all related registers have been updated.
		fpga_module.WriteModule(LBUS::TDC::kAddrSelfHitThreshold, config.hit_threshold);
	}

	bool VerifyTdcSelfTrigger(const SelfTriggerConfig &config, HUL::FPGAModule &fpga_module)
	{
		if (fpga_module.ReadModule(LBUS::TDC::kAddrSelfHitThreshold) != config.hit_threshold)
		{
			return false;
		}

		if (fpga_module.ReadModule(LBUS::TDC::kAddrSelfLatchWindow) != config.latch_window)
		{
			return false;
		}

		if (fpga_module.ReadModule(LBUS::TDC::kAddrSelfGeometryCount) != static_cast<std::uint32_t>(config.geometry_masks.size()))
		{
			return false;
		}

		for (std::uint32_t index = 0; index < LBUS::TDC::kMaxSelfGeometry; ++index)
		{
			const std::uint32_t actual_mask = fpga_module.ReadModule(LBUS::TDC::kAddrSelfGeometryBase + index * LBUS::TDC::kSelfGeometryStride, 4);

			if (actual_mask != GeometryMaskAt(config, index))
			{
				return false;
			}
		}

		return true;
	}

	void SetTdcWindow(std::uint32_t wmax, std::uint32_t wmin, HUL::FPGAModule &fpga_module)
	{
		const std::uint32_t ptr_ofs = kCMax - wmax + kPtrDiffWr;

		fpga_module.WriteModule(LBUS::TDC::kAddrPtrOfs, ptr_ofs, 2);
		fpga_module.WriteModule(LBUS::TDC::kAddrWindowMax, wmax, 2);
		fpga_module.WriteModule(LBUS::TDC::kAddrWindowMin, wmin, 2);
	}
}
