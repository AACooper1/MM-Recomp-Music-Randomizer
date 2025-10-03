#include <iostream>
#include <cstdarg>
#include <sstream>

// Macros
#define START_PARA "\n\n================================================================================\n"
#define END_PARA "\n================================================================================\n\n"

namespace mmrs_util
{
    // Enums
    enum log_level_t
    {
        LOG_NOTHING,
        LOG_CRITICAL,
        LOG_ERROR,
        LOG_WARNING,
        LOG_INFO,
        LOG_DEBUG
    };

    // Variables
    static log_level_t gLogLevel = LOG_NOTHING;

    void set_log_level(log_level_t level);
    std::ostream* _log(log_level_t level);

    std::ostream&  critical();
    std::ostream&  error();
    std::ostream&  warning();
    std::ostream&  info();
    std::ostream&  debug();
}