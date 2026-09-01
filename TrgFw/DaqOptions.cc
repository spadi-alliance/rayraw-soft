#include "DaqOptions.hh"

#include <limits>
#include <ostream>
#include <stdexcept>

namespace HUL::DAQ
{
    namespace
    {
        enum ArgumentIndex
        {
            kProgram,
            kIpAddress,
            kRunNumber,
            kEventCount,
            kWindowMax,
            kWindowMin,
            kFirstOption
        };

        std::int32_t ParseInt32(const char *text, const char *name)
        {
            const std::string value_text(text);
            std::size_t parsed_length = 0;
            long long value = 0;

            try
            {
                value = std::stoll(value_text, &parsed_length, 0);
            }
            catch (const std::exception &)
            {
                throw std::runtime_error("Invalid " + std::string(name) + ": " + value_text);
            }

            if (parsed_length != value_text.size() || value < std::numeric_limits<std::int32_t>::min() || value > std::numeric_limits<std::int32_t>::max())
            {
                throw std::runtime_error("Invalid " + std::string(name) + ": " + value_text);
            }

            return static_cast<std::int32_t>(value);
        }

        std::uint32_t ParseWindow(const char *text, const char *name)
        {
            const std::int32_t value = ParseInt32(text, name);

            if (value < 0)
            {
                throw std::runtime_error(std::string(name) + " must not be negative");
            }

            return static_cast<std::uint32_t>(value);
        }

        std::uint8_t ParseUint8(const std::string &text, const char *name)
        {
            const std::int32_t value = ParseInt32(text.c_str(), name);

            if (value < 0 || value > std::numeric_limits<std::uint8_t>::max())
            {
                throw std::runtime_error(std::string(name) + " must be in the range 0 to 255");
            }

            return static_cast<std::uint8_t>(value);
        }

		void ParseOptionalArgument(const std::string &argument, DaqOptions &options)
        {
            if (argument == "--enable-zerosup" || argument == "--enable_zerosup")
            {
                options.enable_zero_suppression = true;
                return;
            }

			if (argument.rfind("--hit-timeout=", 0) == 0)
            {
                const std::string value_text = argument.substr(14);
				options.hit_timeout = ParseUint8(value_text, "hit timeout");
                return;
			}

            throw std::runtime_error("Unknown option: " + argument);
        }

        void ValidateOptions(const DaqOptions &options)
        {
            const std::uint32_t kMaximumWindow = 2047;

            if (options.window_max > kMaximumWindow)
            {
                throw std::runtime_error("TDC window values must be in the range 0 to 2047");
            }

            if (options.window_min > options.window_max)
            {
                throw std::runtime_error("Min TDC window value must not be larger than Max TDC window value");
            }
        }
    }

    bool IsHelpRequested(int argc, char *argv[])
    {
        if (argc != 2)
        {
            return false;
        }

        const std::string argument(argv[1]);
        return argument == "--help" || argument == "-h";
    }

    bool HasRequiredArguments(int argc)
    {
        return argc >= kFirstOption;
    }

    void PrintUsage(std::ostream &output, const char *program)
    {
        output << "Usage\n";
        output << program << " [IP address] [Run No] [Num of events] [Tdc window max] [Tdc window min] [options]\n";
        output << "\t- Run No            : Run number\n";
        output << "\t- Num of events     : Max event number. DAQ will stop at this event number or by Ctrl-C.\n";
        output << "\t- Tdc window max    : Max TDC window value. (10 ns step)\n";
        output << "\t- Tdc window min    : Min TDC window value. (10 ns step)\n\n";
        output << "Options\n";
        output << "Enable Zero Suppression data acquisition with the following options:\n";
        output << "\t--enable-zerosup\n";
        output << "\t--enable_zerosup\n";
		output << "Set hit timeout value (0-255) with the following option:\n";
		output << "\t--hit-timeout=<value>\n\n";
    }

    DaqOptions ParseDaqOptions(int argc, char *argv[])
    {
        if (!HasRequiredArguments(argc))
        {
            throw std::runtime_error("Insufficient arguments");
        }

        DaqOptions options;
        options.board_ip = argv[kIpAddress];
        options.run_no = ParseInt32(argv[kRunNumber], "run number");
        options.num_event = ParseInt32(argv[kEventCount], "number of events");
        options.window_max = ParseWindow(argv[kWindowMax], "TDC window max");
        options.window_min = ParseWindow(argv[kWindowMin], "TDC window min");
        for (int index = kFirstOption; index < argc; ++index)
        {
            ParseOptionalArgument(argv[index], options);
        }

        ValidateOptions(options);
        return options;
    }
}
