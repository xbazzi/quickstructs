#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

// FastInAHurry Includes
#include "fiah/utils/Logger.hh"
#include "fiah/utils/TomlParser.hh"

namespace fiah::io
{

class Config
{
private:
    std::filesystem::path m_config_path{};
    static inline fiah::utils::Logger<Config>& m_logger{
        fiah::utils::Logger<Config>::get_instance("Config")
    };

    fiah::utils::TomlParser m_parser{};

public:
    Config() noexcept = default;
    Config(const std::filesystem::path&);
    ~Config() = default;

    bool parse_config() noexcept;

    // Server getters
    std::string get_market_ip() const noexcept;
    uint16_t get_market_port() const noexcept;
    // const std::string& get_market_protocol() const noexcept;
};
} // End io namespace
