#ifndef SELF_TRIGGER_CONFIG_HH
#define SELF_TRIGGER_CONFIG_HH

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace HUL::DAQ
{
    struct SelfTriggerConfig
    {
        std::uint32_t hit_threshold{0};
        std::uint32_t latch_window{0};
        std::vector<std::uint32_t> geometry_masks;
    };

    SelfTriggerConfig ParseSelfTriggerConfig(std::istream &input);

    SelfTriggerConfig LoadSelfTriggerConfig(const std::string &filename);

    SelfTriggerConfig LoadSelfTriggerConfig();
}

#endif
