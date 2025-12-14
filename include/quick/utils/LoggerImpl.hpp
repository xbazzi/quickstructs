#include "utils/Logger.hh"

namespace quick::utils
{
static Logger &Logger::get_instance(Location location = Location::current())
{
    static Logger instance{std::move(location)};
    return instance;
}

void Logger::_log(Logger::Level level, const char *func, int line, Args &&...args)
{
    auto now = std::chrono::system_clock::now();
    auto curr_time = std::chrono::system_clock::to_time_t(now);
    auto formatted_time = std::put_time(std::localtime(std::addressof(curr_time)), "%H:%M:%S.00");
    // auto now = std::chrono::utc_clock::to_time_t(now);
    std::ostringstream oss;

    oss << '[' << formatted_time << ']' << '[' << func << ':' << line
        << ']'
        // <<        loc.line()              << ']'
        << '[' << _level_to_string(level) << "] ";

    std::cout << _level_to_color(level) << oss.str();

    (std::cout << ... << std::forward<Args>(args)) << "\033[0m" << std::endl;
}

static const char *Logger::_level_to_string(Level level)
{
    switch (level)
    {
    case Level::INFO:
        return "INFO";
    case Level::DEBUG:
        return "DEBUG";
    case Level::ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

static const char *Logger::_level_to_color(Level level)
{
    switch (level)
    {
    case Level::INFO:
        return "\033[32m"; // Green
    case Level::DEBUG:
        return "\033[36m"; // Cyan
    case Level::ERROR:
        return "\033[31m"; // Red
    default:
        return "\033[0m";
    }
}
} // namespace quick::utils