#pragma once

#include <cstdint>
#include <source_location>
#include <string>
#include <utility>

#include "quick/utils/Logger.hh"

namespace quick::utils
{

// Internal tag type used to instantiate the underlying templated Logger.
struct _InternalLoggerTag
{
};

// Access the underlying Logger singleton for the facade
inline Logger<_InternalLoggerTag> &impl_logger()
{
    // Return the singleton instance. If you want to control the logger name,
    // call `fiah::log::init(name)` before any other logging occurs so the
    // singleton is constructed with your desired name.
    return Logger<_InternalLoggerTag>::get_instance();
}

// Initialize/set the backend logger name
inline void init(std::string name)
{
    // Ensure the Meyers singleton is constructed with the provided name.
    // This must be called before any other call that would construct the
    // singleton (i.e., before the first log call) to take effect.
    (void)Logger<_InternalLoggerTag>::get_instance(std::move(name));
}

// Thin forwarding APIs. These mirror the Logger methods but avoid the need
// for template parameters and instance variables.
template <class... Args>
inline void info(const std::source_location &loc = std::source_location::current(), Args &&...args)
{
    impl_logger().info(loc.function_name(), static_cast<std::uint32_t>(loc.line()), std::forward<Args>(args)...);
}

template <class... Args>
inline void debug(const std::source_location &loc = std::source_location::current(), Args &&...args)
{
    impl_logger().debug(loc.function_name(), static_cast<int>(loc.line()), std::forward<Args>(args)...);
}

template <class... Args>
inline void error(const std::source_location &loc = std::source_location::current(), Args &&...args)
{
    impl_logger().error(loc.function_name(), static_cast<int>(loc.line()), std::forward<Args>(args)...);
}

template <class... Args>
inline void warn(const std::source_location &loc = std::source_location::current(), Args &&...args)
{
    impl_logger().warn(loc.function_name(), static_cast<int>(loc.line()), std::forward<Args>(args)...);
}

// Note: these macros use std::source_location to capture caller info.
#define LOG_INFO_S(...) ::quick::utils::info(std::source_location::current(), __VA_ARGS__)
#define LOG_DEBUG_S(...) ::quick::utils::debug(std::source_location::current(), __VA_ARGS__)
#define LOG_ERROR_S(...) ::quick::utils::error(std::source_location::current(), __VA_ARGS__)
#define LOG_WARN_S(...) ::quick::util::warn(std::source_location::current(), __VA_ARGS__)

} // namespace quick::utils
