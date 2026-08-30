#include "SelfTriggerConfig.hh"

#include "RegisterMap.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifndef RAYRAW_CONFIG_DIR
#define RAYRAW_CONFIG_DIR "."
#endif

namespace HUL::DAQ
{
    namespace
    {
        struct ParseState
        {
            bool threshold_found{false};
            bool latch_window_found{false};
            bool geometry_count_found{false};
            std::size_t expected_geometry_count{0};
            std::size_t line_number{0};
        };

        std::string DefaultConfigPath() { return std::string(RAYRAW_CONFIG_DIR) + "/self_trig.txt"; }

        void RemoveComment(std::string &line)
        {
            const std::size_t comment_position = line.find('#');

            if (comment_position != std::string::npos)
            {
                line.erase(comment_position);
            }
        }

        std::uint64_t ParseUnsigned(const std::string &text, std::size_t line_number)
        {
            std::size_t parsed_length = 0;
            unsigned long long value = 0;

            try
            {
                value = std::stoull(text, &parsed_length, 0);
            }
            catch (const std::exception &)
            {
                throw std::runtime_error("Invalid value at line " + std::to_string(line_number) + ": " + text);
            }

            if (parsed_length != text.size() || (!text.empty() && text[0] == '-'))
            {
                throw std::runtime_error("Invalid value at line " + std::to_string(line_number) + ": " + text);
            }

            return static_cast<std::uint64_t>(value);
        }

        std::uint64_t ReadSingleUnsigned(std::istringstream &line_stream, const std::string &key, std::size_t line_number)
        {
            std::string value_text;

            if (!(line_stream >> value_text))
            {
                throw std::runtime_error("Missing " + key + " value at line " + std::to_string(line_number));
            }

            std::string extra;

            if (line_stream >> extra)
            {
                throw std::runtime_error("Unexpected extra value at line " + std::to_string(line_number) + ": " + extra);
            }

            return ParseUnsigned(value_text, line_number);
        }

        std::uint32_t ParseSettingValue(const std::string &key, std::istringstream &line_stream, std::uint32_t maximum, bool &found, std::size_t line_number)
        {
            if (found)
            {
                throw std::runtime_error("Duplicate " + key + " at line " + std::to_string(line_number));
            }

            const std::uint64_t value = ReadSingleUnsigned(line_stream, key, line_number);

            if (value > maximum)
            {
                throw std::runtime_error(key + " must be 0.." + std::to_string(maximum) + " at line " + std::to_string(line_number));
            }

            found = true;
            return static_cast<std::uint32_t>(value);
        }

        bool ParseScalarSetting(const std::string &key, std::istringstream &line_stream, SelfTriggerConfig &config, ParseState &state)
        {
            if (key == "hit_threshold")
            {
                config.hit_threshold = ParseSettingValue(
                    key,
                    line_stream,
                    LBUS::TDC::kMaxSelfHitThreshold,
                    state.threshold_found,
                    state.line_number);
                return true;
            }

            if (key == "latch_window")
            {
                config.latch_window = ParseSettingValue(
                    key,
                    line_stream,
                    LBUS::TDC::kMaxSelfLatchWindow,
                    state.latch_window_found,
                    state.line_number);
                return true;
            }

            if (key == "num_geometry_conditions")
            {
                state.expected_geometry_count = ParseSettingValue(
                    key,
                    line_stream,
                    LBUS::TDC::kMaxSelfGeometry,
                    state.geometry_count_found,
                    state.line_number);
                return true;
            }

            return false;
        }

        std::uint32_t ParseGeometryMask(std::istringstream &line_stream, std::size_t line_number)
        {
            std::uint32_t mask = 0;
            std::string channel_text;

            while (line_stream >> channel_text)
            {
                const std::uint64_t channel = ParseUnsigned(channel_text, line_number);

                if (channel >= LBUS::TDC::kNumTdcChannels)
                {
                    throw std::runtime_error("TDC channel must be 0.." + std::to_string(LBUS::TDC::kNumTdcChannels - 1) + " at line " + std::to_string(line_number));
                }

                const std::uint32_t channel_bit = std::uint32_t(1) << channel;

                if ((mask & channel_bit) != 0)
                {
                    throw std::runtime_error("Duplicate TDC channel at line " + std::to_string(line_number));
                }

                mask |= channel_bit;
            }

            if (mask == 0)
            {
                throw std::runtime_error("Geometry must contain at least one channel at line " + std::to_string(line_number));
            }

            return mask;
        }

        void ParseGeometry(std::string key, std::istringstream &line_stream, SelfTriggerConfig &config, const ParseState &state)
        {
            if (key.empty() || key.back() != ':')
            {
                throw std::runtime_error("Unknown key at line " + std::to_string(state.line_number) + ": " + key);
            }

            if (!state.geometry_count_found)
            {
                throw std::runtime_error("num_geometry_conditions must precede geometry entries");
            }

            if (config.geometry_masks.size() >= state.expected_geometry_count)
            {
                throw std::runtime_error("Too many geometry conditions at line " + std::to_string(state.line_number));
            }

            key.erase(key.size() - 1);
            const std::uint64_t index = ParseUnsigned(key, state.line_number);

            if (index >= LBUS::TDC::kMaxSelfGeometry)
            {
                throw std::runtime_error("geometry index must be 0.." + std::to_string(LBUS::TDC::kMaxSelfGeometry - 1) + " at line " + std::to_string(state.line_number));
            }

            if (index != config.geometry_masks.size())
            {
                throw std::runtime_error("geometry indices must be contiguous from 0 at line " + std::to_string(state.line_number));
            }

            config.geometry_masks.push_back(ParseGeometryMask(line_stream, state.line_number));
        }

        void ValidateConfig(const SelfTriggerConfig &config, const ParseState &state)
        {
            if (!state.threshold_found)
            {
                throw std::runtime_error("hit_threshold is not specified");
            }

            if (!state.latch_window_found)
            {
                throw std::runtime_error("latch_window is not specified");
            }

            if (!state.geometry_count_found)
            {
                throw std::runtime_error("num_geometry_conditions is not specified");
            }

            if (config.geometry_masks.size() != state.expected_geometry_count)
            {
                throw std::runtime_error("Expected " + std::to_string(state.expected_geometry_count) + " geometry conditions, but found " + std::to_string(config.geometry_masks.size()));
            }
        }

        void PrintConfig(const SelfTriggerConfig &config)
        {
            std::cout << "#D: Hit threshold: " << config.hit_threshold << '\n';
            std::cout << "#D: Latch window: " << config.latch_window << '\n';
            std::cout << "#D: Geometry count: " << config.geometry_masks.size() << '\n';

            for (std::size_t index = 0; index < config.geometry_masks.size(); ++index)
            {
                std::cout << "#D: Geometry " << index << ": 0x" << std::hex << config.geometry_masks[index] << std::dec << '\n';
            }
        }
    }

    SelfTriggerConfig ParseSelfTriggerConfig(std::istream &input)
    {
        SelfTriggerConfig config = SelfTriggerConfig();
        ParseState state;
        std::string line;

        while (std::getline(input, line))
        {
            ++state.line_number;
            RemoveComment(line);

            std::istringstream line_stream(line);
            std::string key;

            if (!(line_stream >> key))
            {
                continue;
            }

            if (ParseScalarSetting(key, line_stream, config, state))
            {
                continue;
            }

            ParseGeometry(key, line_stream, config, state);
        }

        if (input.bad())
        {
            throw std::runtime_error("Failed while reading Self Trigger config");
        }

        ValidateConfig(config, state);
        return config;
    }

    SelfTriggerConfig LoadSelfTriggerConfig(const std::string &filename)
    {
        std::ifstream input(filename.c_str());

        if (!input.is_open())
        {
            throw std::runtime_error("Cannot open Self Trigger config: " + filename);
        }

        std::cout << "#D: Self Trigger config: " << filename << '\n';
        SelfTriggerConfig config = ParseSelfTriggerConfig(input);
        PrintConfig(config);
        return config;
    }

    SelfTriggerConfig LoadSelfTriggerConfig()
    {
        return LoadSelfTriggerConfig(DefaultConfigPath());
    }

}
