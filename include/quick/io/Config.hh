#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

// FastInAHurry Includes
#include "quick/utils/Logger.hh"
#include "quick/utils/TomlParser.hh"

namespace quick::io
{

class Config
{
  private:
    std::filesystem::path m_config_path{};
    static inline quick::utils::Logger<Config> &m_logger{quick::utils::Logger<Config>::get_instance("Config")};

    quick::utils::TomlParser m_parser{};

  public:
    Config() noexcept = default;
    Config(const std::filesystem::path &path) : m_config_path{path}, m_parser{path}
    {
    }
    ~Config() = default;

    bool parse_config() noexcept
    {
        return m_parser.load()
            .transform([](bool success) {
                LOG_INFO("TOML file loaded successfully.");
                return success;
            })
            .or_else([](error::TomlParserError error) -> std::expected<bool, error::TomlParserError> {
                LOG_ERROR("Failed to load TOML file: ", static_cast<int>(error));
                return std::unexpected(error);
            })
            .value_or(false);
    }

    // Server getters
    std::string get_market_ip() const noexcept
    {
        // get_value returns std::optional<std::string>
        // return a safe copy (empty string if not found)
        return m_parser.get_value("network", "ip").value_or(std::string{});
    }
    uint16_t get_market_port() const noexcept
    {
        // Read once and handle parse errors locally to avoid throwing from noexcept
        auto opt = m_parser.get_value("network", "port");
        if (!opt.has_value())
        {
            return 0;
        }

        try
        {
            const auto val = std::stoi(*opt);
            if (val < 0)
                return 0;
            return static_cast<uint16_t>(val);
        }
        catch (const std::exception &)
        {
            // stoi can throw invalid_argument or out_of_range; treat as
            // missing/invalid
            return 0;
        }
    }
    // const std::string& get_market_protocol() const noexcept;
};

} // namespace quick::io
