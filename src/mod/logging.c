// MM Recomp Mod Requirements
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "logging.h"

#include "libc64/sprintf.h"

RECOMP_IMPORT(".", void _log(const unsigned char* msg, int logLevel))

typedef enum LogLevel
{
    LOG_NOTHING,
    LOG_CRITICAL,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_DEV
};

void _dev(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    vsprintf(dest, fmt, args);
    _log(dest, LOG_DEV);
    va_end(args);
}

void _debug(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    vsprintf(dest, fmt, args);
    _log(dest, LOG_DEBUG);
    va_end(args);
}


void _info(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    vsprintf(dest, fmt, args);
    _log(dest, LOG_INFO);
    va_end(args);
}

void _warning(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    vsprintf(dest, fmt, args);
    _log(dest, LOG_WARNING);
    va_end(args);
}

void _error(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    vsprintf(dest, fmt, args);
    _log(dest, LOG_ERROR);
    va_end(args);
}

void _critical(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    vsprintf(dest, fmt, args);
    _log(dest, LOG_CRITICAL);
    va_end(args);
}

void logger_init(struct Logger* logger)
{
    logger->debug = _debug;
}