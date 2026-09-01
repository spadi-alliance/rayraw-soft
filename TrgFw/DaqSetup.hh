#ifndef DAQ_SETUP_HH
#define DAQ_SETUP_HH

namespace HUL
{
    class FPGAModule;
}

namespace HUL::DAQ
{
    struct DaqOptions;
    struct SelfTriggerConfig;

    void ConfigureHardware(HUL::FPGAModule &fpga_module, const DaqOptions &options, const SelfTriggerConfig &self_trigger_config);
}

#endif
