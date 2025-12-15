/**
 * @file TomlParser.hh
 * @author Xander Bazzi (codemaster@xbazzi.com)
 * @brief
 * @version 0.1
 * @date 2025-11-08
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once
// C++ Includes
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// QuickLib Includes
#include "quick/utils/Logger.hh"

// Third Party Includes

// QUICK Includes
#include "quick/error/Error.hh"

namespace quick::utils
{
/**
 * @brief Simple TOML parser for configuration files
 *
 */
class TomlParser
{
  private:
    // std::ifstream m_ifs;
    std::filesystem::path m_filepath;
    std::vector<std::string> m_sections;
    static inline Logger<TomlParser> &m_logger{Logger<TomlParser>::get_instance("TomlParser")};

    // Map of section -> (key -> value)
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_keys;

  public:
    TomlParser() noexcept;
    TomlParser(const std::filesystem::path &path) noexcept;

    ~TomlParser() = default;

    /** @brief Validate the TOML file
     *  @param file The input file stream
     *  @return true if valid, false otherwise
     */
    std::expected<bool, quick::error::TomlParserError> is_valid(std::ifstream &file) noexcept;

    std::expected<bool, quick::error::TomlParserError> extract_keys(std::ifstream &file) noexcept;

    /** @brief Load a TOML file from disk
     *  @return true if successful, false otherwise
     */
    std::expected<bool, quick::error::TomlParserError> load() noexcept;

    /** @brief Get a value from the TOML file as a string
     *  @param key The key to look up
     *  @return The value as a string
     */
    std::string get_string(const std::string &key) const;

    /** @brief Get a value from the TOML file as an integer
     *  @param key The key to look up
     *  @return The value as an integer
     */
    int get_int(const std::string &key) const;

    /** @brief Get a value from the TOML file as a double
     *  @param key The key to look up
     *  @return The value as a double
     */
    double get_double(const std::string &key) const;

    std::optional<std::string> get_value(const std::string &section, const std::string &key) const;
};

// ============================================================================
// Inline Implementation
// ============================================================================

inline TomlParser::TomlParser() noexcept : m_filepath{}
{
    m_filepath = "etc/config.toml";
}

inline TomlParser::TomlParser(const std::filesystem::path &path) noexcept : m_filepath{path}
{
}

inline std::optional<std::string> TomlParser::get_value(const std::string &section, const std::string &key) const
{
    auto section_it = m_keys.find(section);
    if (section_it == std::end(m_keys))
    {
        LOG_DEBUG("Could not find section");
        return std::nullopt;
    }

    auto key_it = section_it->second.find(key);
    if (key_it == section_it->second.end())
    {
        LOG_DEBUG("Could not find key");
        return std::nullopt;
    }
    LOG_DEBUG("Found key with value: ", key_it->second);

    return key_it->second;
}

inline auto TomlParser::load() noexcept -> std::expected<bool, quick::error::TomlParserError>
{
    std::ifstream file{m_filepath};
    if (!file.is_open())
    {
        return std::unexpected(quick::error::TomlParserError::FILE_NOT_FOUND);
    }

    auto placeholder = true;
    if (!placeholder)
    {
        return std::unexpected(quick::error::TomlParserError::INVALID_TOML);
    }

    auto result = extract_keys(file);
    if (!result)
    {
        return std::unexpected(quick::error::TomlParserError::PARSE_ERROR);
    }

    file.close();
    return true;
}

inline auto TomlParser::extract_keys(std::ifstream &file) noexcept -> std::expected<bool, quick::error::TomlParserError>
{
    std::string line;
    std::string current_section;
    while (std::getline(file, line))
    {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        if (line.empty())
            continue;
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.size() > 0 && line[0] == '#')
            continue;

        LOG_DEBUG("Processing line from TOML file: ", std::quoted(line));

        auto left_bracket = line.find('[');
        if (left_bracket != std::string::npos)
        {
            auto section = line.substr(left_bracket + 1);
            section = section.substr(0, section.find(']'));
            section.erase(0, section.find_first_not_of(" \t\r\n"));
            section.erase(section.find_last_not_of(" \t\r\n") + 1);
            current_section = std::move(section);
            m_sections.push_back(current_section);
            continue;
        }

        auto equal_sign = line.find('=');
        if (equal_sign == std::string::npos)
        {
            return std::unexpected(quick::error::TomlParserError::PARSE_ERROR);
        }

        if (current_section.empty())
        {
            return std::unexpected(quick::error::TomlParserError::PARSE_ERROR);
        }

        std::string key = line.substr(0, equal_sign);
        std::string val = line.substr(equal_sign + 1);

        LOG_DEBUG("Parsed key raw: ", key, " with value: ", val);

        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        val.erase(0, val.find_first_not_of(" \t\r\n"));
        val.erase(val.find_last_not_of(" \t\r\n") + 1);

        auto comment_pos = val.find('#');
        if (comment_pos != std::string::npos)
        {
            val = val.substr(0, comment_pos);
            val.erase(val.find_last_not_of(" \t\r\n") + 1);
            val.erase(0, val.find_first_not_of(" \t\r\n"));
        }

        if (val.size() >= 2 &&
            ((val.front() == '"' && val.back() == '"') || (val.front() == '\'' && val.back() == '\'')))
        {
            val = val.substr(1, val.size() - 2);
        }

        LOG_DEBUG("Parsed key after removing space/comments/quotes: ", key, " with value: ", val);

        m_keys[current_section].emplace(std::move(key), std::move(val));
    }
    return true;
}

inline auto TomlParser::is_valid(std::ifstream &file) noexcept -> std::expected<bool, quick::error::TomlParserError>
{
    return std::unexpected(quick::error::TomlParserError::INVALID_TOML);
}

inline std::string TomlParser::get_string(const std::string &key) const
{
    auto dot = key.find('.');
    if (dot == std::string::npos)
    {
        return {};
    }
    auto section = key.substr(0, dot);
    auto subkey = key.substr(dot + 1);

    auto sit = m_keys.find(section);
    if (sit == m_keys.end())
        return {};

    auto kit = sit->second.find(subkey);
    if (kit == sit->second.end())
        return {};

    std::string val = kit->second;
    if (val.size() >= 2 && ((val.front() == '"' && val.back() == '"') || (val.front() == '\'' && val.back() == '\'')))
    {
        return val.substr(1, val.size() - 2);
    }
    return val;
}

inline int TomlParser::get_int(const std::string &key) const
{
    auto s = get_string(key);
    if (s.empty())
        return 0;
    try
    {
        return std::stoi(s);
    }
    catch (...)
    {
        return 0;
    }
}

inline double TomlParser::get_double(const std::string &key) const
{
    auto s = get_string(key);
    if (s.empty())
        return 0.0;
    try
    {
        return std::stod(s);
    }
    catch (...)
    {
        return 0.0;
    }
}

} // namespace quick::utils