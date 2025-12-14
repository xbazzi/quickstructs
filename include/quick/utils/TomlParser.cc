/**
 * @file TomlParser.cc
 * @author Xander Bazzi (codemaster@xbazzi.com)
 * @brief
 * @date 2025-11-08
 *
 * @copyright Copyright (c) 2025
 *
 */

// C++ Includes
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>

// Third Party Includes
#include <toml.hpp>

// FastInAHurry Includes
#include "fiah/Error.hh"
#include "fiah/utils/Logger.hh"
#include "fiah/utils/TomlParser.hh"

namespace fiah::utils
{

TomlParser::TomlParser() noexcept : m_filepath{}
{
    // m_ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    m_filepath = "etc/config.toml";
}

TomlParser::TomlParser(const std::filesystem::path &path) noexcept : m_filepath{path}
{
    // m_ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
}

std::optional<std::string> TomlParser::get_value(const std::string &section, const std::string &key) const
{
    auto section_it = m_keys.find(section);
    if (section_it == std::end(m_keys))
    {
        LOG_DEBUG("Could not find section");
        return std::nullopt;
    }

    // find the key inside the section map
    auto key_it = section_it->second.find(key);
    if (key_it == section_it->second.end())
    {
        LOG_DEBUG("Could not find key");
        return std::nullopt;
    }
    LOG_DEBUG("Found key with value: ", key_it->second);

    return key_it->second; // copy into optional
}

auto TomlParser::load() noexcept -> std::expected<bool, TomlParserError>
{
    std::ifstream file{m_filepath};
    if (!file.is_open())
    {
        return std::unexpected(TomlParserError::FILE_NOT_FOUND);
    }

    /// @todo implement is_valid
    // auto result = is_valid(file);
    auto placeholder = true;
    if (!placeholder)
    {
        return std::unexpected(TomlParserError::INVALID_TOML);
    }

    auto result = extract_keys(file);
    if (!result)
    {
        return std::unexpected(TomlParserError::PARSE_ERROR);
    }

    file.close();
    return true;
}

auto TomlParser::extract_keys(std::ifstream &file) noexcept -> std::expected<bool, TomlParserError>
{
    // Extract root keys from toml file
    std::string line;
    std::string current_section;
    while (std::getline(file, line))
    {
        // Trim leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        if (line.empty())
            continue;
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip comments
        if (line.size() > 0 && line[0] == '#')
            continue;

        LOG_DEBUG("Processing line from TOML file: ", std::quoted(line));

        auto left_bracket = line.find('[');
        if (left_bracket != std::string::npos)
        {
            // Section header
            auto section = line.substr(left_bracket + 1);
            section = section.substr(0, section.find(']'));
            // Trim whitespace from section
            section.erase(0, section.find_first_not_of(" \t\r\n"));
            section.erase(section.find_last_not_of(" \t\r\n") + 1);
            current_section = std::move(section);
            m_sections.push_back(current_section);
            continue;
        }

        // Key=value line
        auto equal_sign = line.find('=');
        if (equal_sign == std::string::npos)
        {
            // Not a section and not a key/value -> skip or treat as error
            return std::unexpected(TomlParserError::PARSE_ERROR);
        }

        if (current_section.empty())
        {
            // Keys must belong to a section in this simple parser
            return std::unexpected(TomlParserError::PARSE_ERROR);
        }

        std::string key = line.substr(0, equal_sign);
        std::string val = line.substr(equal_sign + 1);

        LOG_DEBUG("Parsed key raw: ", key, " with value: ", val);

        // Trim leading/trailing whitespace from key and value first
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        val.erase(0, val.find_first_not_of(" \t\r\n"));
        val.erase(val.find_last_not_of(" \t\r\n") + 1);

        // Remove inline comment from value (simple heuristic): strip from first
        // '#' to end. This parser does not handle '#' inside quoted strings.
        auto comment_pos = val.find('#');
        if (comment_pos != std::string::npos)
        {
            val = val.substr(0, comment_pos);
            // trim again after removing comment
            val.erase(val.find_last_not_of(" \t\r\n") + 1);
            val.erase(0, val.find_first_not_of(" \t\r\n"));
        }

        // remove quotes from value if present
        if (val.size() >= 2 &&
            ((val.front() == '"' && val.back() == '"') || (val.front() == '\'' && val.back() == '\'')))
        {
            val = val.substr(1, val.size() - 2);
        }

        LOG_DEBUG("Parsed key after removing space/comments/quotes: ", key, " with value: ", val);

        // Insert into map
        // Prefer emplace to move the strings into the map
        m_keys[current_section].emplace(std::move(key), std::move(val));
    }
    return true;
}

auto TomlParser::is_valid(std::ifstream &file) noexcept -> std::expected<bool, TomlParserError>
{
    return std::unexpected(TomlParserError::INVALID_TOML);
}

std::string TomlParser::get_string(const std::string &key) const
{
    // Expect keys in the form "section.key"
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

    // Strip surrounding quotes if present
    std::string val = kit->second;
    if (val.size() >= 2 && ((val.front() == '"' && val.back() == '"') || (val.front() == '\'' && val.back() == '\'')))
    {
        return val.substr(1, val.size() - 2);
    }
    return val;
}

int TomlParser::get_int(const std::string &key) const
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

double TomlParser::get_double(const std::string &key) const
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

} // namespace fiah::utils