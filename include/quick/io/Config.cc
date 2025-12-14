#include <filesystem>
#include <iostream>

// #include <toml.hpp>

#include "fiah/io/Config.hh"
#include "fiah/utils/Timer.hpp"

namespace fiah::io
{
Config::Config(const std::filesystem::path &path) : m_config_path{path}, m_parser{path}
{
}

bool Config::parse_config() noexcept
{
    return m_parser.load()
        .transform([](bool success) {
            LOG_INFO("TOML file loaded successfully.");
            return success;
        })
        .or_else([](TomlParserError error) -> std::expected<bool, TomlParserError> {
            LOG_ERROR("Failed to load TOML file: ", static_cast<int>(error));
            return std::unexpected(error);
        })
        .value_or(false);
}

std::string Config::get_market_ip() const noexcept
{
    // get_value returns std::optional<std::string>
    // return a safe copy (empty string if not found)
    return m_parser.get_value("network", "ip").value_or(std::string{});
}

uint16_t Config::get_market_port() const noexcept
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
} // namespace fiah::io