#ifndef DAQ_OPTIONS_HH
#define DAQ_OPTIONS_HH

#include <cstdint>
#include <iosfwd>
#include <string>

namespace HUL::DAQ
{
    struct DaqOptions
    {
        std::string board_ip;
        std::int32_t run_no{0};
        std::int32_t num_event{0};
        std::uint32_t window_max{0};
        std::uint32_t window_min{0};
        bool enable_zero_suppression{false};
		std::uint8_t hit_timeout{200};
    };

    bool IsHelpRequested(int argc, char *argv[]);

    bool HasRequiredArguments(int argc);

    void PrintUsage(std::ostream &output, const char *program);

    DaqOptions ParseDaqOptions(int argc, char *argv[]);
}

#endif
