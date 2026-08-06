#include "MhTdcFuncs.hh"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#ifndef RAYRAW_CONFIG_DIR
#define RAYRAW_CONFIG_DIR "."
#endif

namespace HUL::DAQ
{
	namespace
	{
		constexpr uint64_t kMaxHitThreshold       = 32;
		constexpr uint64_t kMaxLatchWindow        = 15;
		constexpr uint64_t kMaxGeometryConditions = 16;
		constexpr uint64_t kNumTdcChannels        = 32;

		std::filesystem::path FindSelfTriggerConfig()
		{
			const std::filesystem::path local_path = std::filesystem::path("config") / "self_trig.txt";

			if (std::filesystem::is_regular_file(local_path))
			{
				return local_path;
			}

			throw std::runtime_error("self_trig.txt was not found (searched " + local_path.string() + ")");
		}

		uint64_t ParseUnsigned(const std::string &text, std::size_t line_number)
		{
			std::size_t parsed_length = 0;

			try
			{
				const uint64_t value = std::stoull(text, &parsed_length, 0);

				if (parsed_length != text.size())
				{
					throw std::invalid_argument("trailing characters");
				}

				return value;
			}
			catch (const std::exception &)
			{
				throw std::runtime_error("Invalid value at line " + std::to_string(line_number) + ": " + text);
			}
		}

		std::string ReadValue(std::istringstream &line_stream, const std::string &key, std::size_t line_number)
		{
			std::string value;

			if (!(line_stream >> value))
			{
				throw std::runtime_error("Missing " + key + " value at line " + std::to_string(line_number));
			}

			return value;
		}

		void RejectExtraValue(std::istringstream &line_stream, std::size_t line_number)
		{
			std::string extra;

			if (line_stream >> extra)
			{
				throw std::runtime_error("Unexpected value at line " + std::to_string(line_number) + ": " + extra);
			}
		}
	}

	SelfTriggerConfig LoadSelfTriggerConfig()
	{
		const std::filesystem::path filename = FindSelfTriggerConfig();

		std::cout << "#D: Self Trigger config: " << std::filesystem::absolute(filename) << std::endl;

		std::ifstream input(filename);

		if (!input.is_open())
		{
			throw std::runtime_error("Cannot open Self Trigger config: " + filename.string());
		}

		SelfTriggerConfig config;
		bool threshold_found = false;
		bool latch_window_found = false;
		bool geometry_count_found = false;
		std::size_t expected_geometry_count = 0;
		std::string line;
		std::size_t line_number = 0;

		while (std::getline(input, line))
		{
			++line_number;

			const std::size_t comment_position = line.find('#');

			if (comment_position != std::string::npos)
			{
				line.erase(comment_position);
			}

			std::istringstream line_stream(line);
			std::string key;

			if (!(line_stream >> key))
			{
				continue;
			}

			if (key == "hit_threshold")
			{
				if (threshold_found)
				{
					throw std::runtime_error("Duplicate hit_threshold at line " + std::to_string(line_number));
				}

				const uint64_t value =
					ParseUnsigned(ReadValue(line_stream, key, line_number), line_number);

				if (value > kMaxHitThreshold)
				{
					throw std::runtime_error("hit_threshold must be 0..32");
				}

				RejectExtraValue(line_stream, line_number);
				config.hit_threshold = static_cast<uint32_t>(value);
				threshold_found = true;
				continue;
			}

			if (key == "latch_window")
			{
				if (latch_window_found)
				{
					throw std::runtime_error("Duplicate latch_window at line " + std::to_string(line_number));
				}

				const uint64_t value =
					ParseUnsigned(ReadValue(line_stream, key, line_number), line_number);

				if (value > kMaxLatchWindow)
				{
					throw std::runtime_error("latch_window must be 0..15");
				}

				RejectExtraValue(line_stream, line_number);
				config.latch_window = static_cast<uint32_t>(value);
				latch_window_found = true;
				continue;
			}

			if (key == "num_geometry_conditions")
			{
				if (geometry_count_found)
				{
					throw std::runtime_error("Duplicate num_geometry_conditions at line " + std::to_string(line_number));
				}

				const uint64_t count =
					ParseUnsigned(ReadValue(line_stream, key, line_number), line_number);

				if (count > kMaxGeometryConditions)
				{
					throw std::runtime_error("num_geometry_conditions must be 0..16");
				}

				RejectExtraValue(line_stream, line_number);
				expected_geometry_count = static_cast<std::size_t>(count);
				geometry_count_found = true;
				continue;
			}

			if (key.back() != ':')
			{
				throw std::runtime_error("Unknown key at line " + std::to_string(line_number) + ": " + key);
			}

			if (!geometry_count_found)
			{
				throw std::runtime_error("num_geometry_conditions must precede geometry entries");
			}

			if (config.geometry_masks.size() >= expected_geometry_count)
			{
				throw std::runtime_error("Too many geometry conditions at line " + std::to_string(line_number));
			}

			key.pop_back();
			const uint64_t index = ParseUnsigned(key, line_number);

			if (index >= kMaxGeometryConditions)
			{
				throw std::runtime_error("geometry index must be 0..15");
			}

			if (index != config.geometry_masks.size())
			{
				throw std::runtime_error("geometry indices must be contiguous from 0 at line " + std::to_string(line_number));
			}

			uint32_t mask = 0;
			std::string channel_text;

			while (line_stream >> channel_text)
			{
				const uint64_t channel = ParseUnsigned(channel_text, line_number);

				if (channel >= kNumTdcChannels)
				{
					throw std::runtime_error("TDC channel must be 0..31 at line " + std::to_string(line_number));
				}

				const uint32_t channel_bit = uint32_t{1} << channel;

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

			config.geometry_masks.push_back(mask);
		}

		if (input.bad())
		{
			throw std::runtime_error("Failed while reading Self Trigger config: " + filename.string());
		}

		if (!threshold_found)
		{
			throw std::runtime_error("hit_threshold is not specified");
		}

		if (!latch_window_found)
		{
			throw std::runtime_error("latch_window is not specified");
		}

		if (!geometry_count_found)
		{
			throw std::runtime_error("num_geometry_conditions is not specified");
		}

		if (config.geometry_masks.size() != expected_geometry_count)
		{
			throw std::runtime_error("Expected " + std::to_string(expected_geometry_count) + " geometry conditions, but found " + std::to_string(config.geometry_masks.size()));
		}

		std::cout << "#D: Hit threshold: " << config.hit_threshold << '\n';
		std::cout << "#D: Latch window: " << config.latch_window << '\n';
		std::cout << "#D: Geometry count: " << config.geometry_masks.size() << '\n';

		for (std::size_t i = 0; i < config.geometry_masks.size(); ++i)
		{
			std::cout << "#D: Geometry " << i << ": 0x" << std::hex << config.geometry_masks[i] << std::dec << '\n';
		}

		return config;
	}
}

