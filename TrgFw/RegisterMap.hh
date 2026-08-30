#ifndef REGISTER_MAP_TRGFW_HH
#define REGISTER_MAP_TRGFW_HH

#include <stdint.h>

namespace LBUS
{
	//-------------------------------------------------------------------------
	// TRM Module
	//-------------------------------------------------------------------------
	namespace TRM
	{
		enum LocalAddress
		{
			kAddrSelectTrigger = 0x10000000 // W/R, [15:0] select trigger line
		};

		enum command_sel_trig
		{
			kRegL1Ext   = 0x1,
			kRegL1J0    = 0x2,
			kRegL1RM    = 0x4,
			kRegL1Self  = 0x8,
			kRegL2Ext   = 0x10,
			kRegL2J0    = 0x20,
			kRegL2RM    = 0x40,
			kRegL2Self  = 0x80,
			kRegClrExt  = 0x100,
			kRegClrJ0   = 0x200,
			kRegClrRM   = 0x400,
			kRegClrSelf = 0x800,
			kRegEnL2    = 0x1000,
			kRegEnJ0    = 0x2000,
			kRegEnRM    = 0x4000,
			kRegEnSelf  = 0x8000
		};
	};

	//-------------------------------------------------------------------------
	// DCT Module
	//-------------------------------------------------------------------------
	namespace DCT
	{
		enum LocalAddress
		{
			kAddrDaqGate  = 0x20000000, // W/R, [0:0] Set DAQ Gate reg
			kAddrResetEvb = 0x20100000  // W,   Assert EVB reset (self counter reset)
		};
	};

	//-------------------------------------------------------------------------
	// TDC Module
	//-------------------------------------------------------------------------
	namespace TDC
	{
		enum LocalAddress
		{
			kAddrEnableBlock = 0x30000000, // W/R, [7:0]  Block enable (0:3-Leading, 4:7-Trailing)
			kAddrPtrOfs      = 0x30100000, // W/R, [10:0] Offset read pointer
			kAddrWindowMax   = 0x30200000, // W/R, [10:0] Search window max
			kAddrWindowMin   = 0x30300000, // W/R, [10:0] Search window min

			kAddrSelfHitThreshold  = 0x30400000, // W/R, [5:0] Hit threshold
			kAddrSelfLatchWindow   = 0x30500000, // W/R, [3:0] valid hit time (×13.3 ns)
			kAddrSelfGeometryCount = 0x30600000, // W/R, [4:0] number of valid geometry condition
			kAddrSelfGeometryBase  = 0x31000000	 // W/R, [15:0][31:0] geometry condition array
		};

		static constexpr uint32_t kSelfGeometryStride  = 0x00100000;
		static constexpr uint32_t kNumTdcChannels      = 32;
		static constexpr uint32_t kMaxSelfGeometry     = 16;
		static constexpr uint32_t kMaxSelfHitThreshold = 32;
		static constexpr uint32_t kMaxSelfLatchWindow  = 15;

		enum TdcBlock
		{
			kEnLeading  = 0x1,
			kEnTrailing = 0x2
		};
	};

	//-------------------------------------------------------------------------
	// IOM Module
	//-------------------------------------------------------------------------
	namespace IOM
	{
		enum LocalAddress
		{
			kAddrNimout1 = 0x40000000, // W/R, [3:0]
			kAddrNimout2 = 0x40100000, // W/R, [3:0]
			kAddrNimout3 = 0x40200000, // W/R, [3:0]
			kAddrNimout4 = 0x40300000, // W/R, [3:0]
			kAddrExtL1   = 0x40400000, // W/R, [2:0]
			kAddrExtL2   = 0x40500000, // W/R, [2:0]
			kAddrExtClr  = 0x40600000, // W/R, [2:0]
			kAddrExtBusy = 0x40700000, // W/R, [2:0]
			kAddrExtRsv2 = 0x40800000  // W/R, [2:0]
		};

		enum OutputSubbAddress
		{
			kReg_o_ModuleBusy = 0x0,
			kReg_o_CrateBusy  = 0x1,
			kReg_o_RML1       = 0x2,
			kReg_o_RML2       = 0x3,
			kReg_o_RMClr      = 0x4,
			kReg_o_RMRsv1     = 0x5,
			kReg_o_RMSnInc    = 0x6,
			kReg_o_DaqGate    = 0x7,
			kReg_o_DIP8       = 0x8,
			kReg_o_clk1MHz    = 0x9,
			kReg_o_clk100kHz  = 0xA,
			kReg_o_clk10kHz   = 0xB,
			kReg_o_clk1kHz    = 0xC,
			kReg_o_NC         = 0xE,
			kReg_o_Default    = 0xF
		};

		enum InputSubbAddress
		{
			kReg_i_Nimin1  = 0x0,
			kReg_i_Nimin2  = 0x1,
			kReg_i_Nimin3  = 0x2,
			kReg_i_Nimin4  = 0x3,
			kReg_i_NC      = 0x6,
			kReg_i_Default = 0x7
		};
	};

	//-------------------------------------------------------------------------
	// ADC Module
	//-------------------------------------------------------------------------
	namespace ADC
	{
		enum LocalAddress
		{
			kAddrPtrOfs        = 0x50000000,   // W/R, [10:0] Offset read pointer
			kAddrWindowMax     = 0x50100000,	 // W/R, [10:0] Search window max
			kAddrWindowMin     = 0x50200000,	 // W/R, [10:0] Search window min
			kAddrAdcRoReset    = 0x50300000,	 // W/R, [0:0]  AdcRo reset signal
			kAddrAdcRoIsReady  = 0x50400000,	 // R,   [3:0]  AdcRo IsReady signals
			kAddrEnableZeroSup = 0x50500000,	 // W/R, [0:0]  Enable Zero Suppression
			kAddrHitTimeout    = 0x50600000,     // W/R, [7:0]  Hit Timeout
		};
	};

};

#endif
