#ifndef LOGGER_HH
#define LOGGER_HH

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <mutex>
#include <source_location>
#include <sstream>
#include <string>
#include <type_traits>

namespace quick::utils
{

/// @brief Thread-safe (locking) logging implementation. You need to provide a
/// `Tag` template parameter
///        (usually just the class name that needs this Logger)to instantiate a
///        maximum of one `Logger` per class. Uses Meyer's Singleton pattern.
/// @attention Instances must be exactly named `m_logger` for the
///            macros to work properly (`LOG_INFO()`, `LOG_ERROR()`, etc...)
/// @tparam Tag
/// @warning Made u look, u good tho.
template <class Tag> class Logger
{
  public:
    enum class Level : std::uint8_t
    {
        INFO,
        DEBUG,
        ERROR,
        WARN
    };

    using Location = std::source_location;

    /// @brief Singleton-returning method
    /// @param location
    /// @return Singleton per name, which is the class that uses this Logger
    __always_inline static Logger &get_instance(std::string &&name = "GenericLogger")
    {
        static Logger instance{std::move(name)};
        return instance;
    }

    template <class... Args> void info(const char *func, int line, Args &&...args)
    {
        _log(Level::INFO, func, line, std::forward<Args>(args)...);
    }

    template <class... Args> void error(const char *func, int line, Args &&...args)
    {
        _log(Level::ERROR, func, line, std::forward<Args>(args)...);
    }

    template <class... Args> void debug(const char *func, int line, Args &&...args)
    {
        _log(Level::DEBUG, func, line, std::forward<Args>(args)...);
    }

    template <class... Args> void warn(const char *func, int line, Args &&...args)
    {
        _log(Level::WARN, func, line, std::forward<Args>(args)...);
    }

#define LOG_INFO(...) m_logger.info(__func__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) m_logger.debug(__func__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) m_logger.error(__func__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) m_logger.warn(__func__, __LINE__, __VA_ARGS__)

    /// @todo Research this. Could potentially break stuff
    // friend std::ostream &operator<<(std::ostream &os, const Logger &rhs) {
    //     os << "name: " << rhs.m_scope;
    //     return os;
    // }
    ~Logger()
    {
        // Destructor
    }

  private:
    std::string m_name;
    mutable std::mutex m_log_mutex;

    Logger(std::string &&name) : m_name{std::move(name)}
    {
    }

    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;
    Logger &operator=(const Logger &) = delete;

    /// @brief Thread-safe logging implementation
    /// @param level    Log level
    /// @param ...args  Stuff to log (must implement <<operator)
    ///
    /// @todo Implement `has_stream_operator` concept to assert << availability
    template <class... Args> void _log(Logger::Level level, const char *func, int line, Args &&...args)
    {
        // Get timestamp
        auto now = std::chrono::system_clock::now();
        auto curr_time = std::chrono::system_clock::to_time_t(now);
        auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch() % std::chrono::seconds(1));

        // Thread-safe time formatting
        std::tm time_buf{};

        /// At least I'm giving you logging on Windows so you can
        /// figure out what's wrong, cuz nothing else is guaranteed to work.
#ifdef _WIN32
        localtime_s(&time_buf, &curr_time);
#else
        localtime_r(&curr_time, &time_buf);
#endif

        // Build entire message in a single stringstream
        std::ostringstream oss;
        oss << _level_to_color(level) << '[';

        // Manual time formatting to avoid std::put_time issues
        oss << (time_buf.tm_hour < 10 ? "0" : "") << time_buf.tm_hour << ':' << (time_buf.tm_min < 10 ? "0" : "")
            << time_buf.tm_min << ':' << (time_buf.tm_sec < 10 ? "0" : "") << time_buf.tm_sec << '.' << ms.count()
            << ']' << '[' << _level_to_string(level) << ']' << '[' << m_name << "::" << func << ':' << line << "] ";

        // Append all arguments
        (oss << ... << std::forward<Args>(args));
        oss << "\033[0m" << '\n';

        // Single atomic write to stdout with mutex protection
        {
            std::lock_guard<std::mutex> lock(m_log_mutex);
            std::cout << oss.str() << std::flush;
        }
    }

    static const char *_level_to_string(Level level)
    {
        switch (level)
        {
        case Level::INFO:
            return "INFO";
        case Level::DEBUG:
            return "DEBUG";
        case Level::ERROR:
            return "ERROR";
        case Level::WARN:
            return "WARN";
        default:
            return "UNKNOWN";
        }
    }

    static const char *_level_to_color(Level level)
    {
        switch (level)
        {
        case Level::INFO:
            return "\033[32m"; // Green
        case Level::DEBUG:
            return "\033[36m"; // Cyan
        case Level::ERROR:
            return "\033[31m"; // Red
        case Level::WARN:
            return "\033[33m"; // Red
        default:
            return "\033[0m";
        }
    }
};
} // End namespace quick::utils

#endif // LOGGER_HH
