#include "logging.h"

RECOMP_IMPORT(".", void _log(const unsigned char* msg, int logLevel))
RECOMP_IMPORT(".", void _set_log_level(LogLevel level));
RECOMP_IMPORT(".", int _get_log_level());

void _dev(const char* fmt, ...);
void _debug(const char* fmt, ...);
void _info(const char* fmt, ...);
void _warning(const char* fmt, ...);
void _error(const char* fmt, ...);
void _critical(const char* fmt, ...);

void _dev_noheader(const char* fmt, ...);
void _debug_noheader(const char* fmt, ...);
void _info_noheader(const char* fmt, ...);
void _warning_noheader(const char* fmt, ...);
void _error_noheader(const char* fmt, ...);
void _critical_noheader(const char* fmt, ...);

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

void _dev_noheader(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    if (get_log_level() >= LOG_DEV)
    {
        vsprintf(dest, fmt, args);
        recomp_printf(dest);
    }
    va_end(args);
}

void _debug_noheader(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    if (get_log_level() >= LOG_DEBUG)
    {
        vsprintf(dest, fmt, args);
        recomp_printf(dest);
    }
    va_end(args);
}

void _info_noheader(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    if (get_log_level() >= LOG_INFO)
    {
        vsprintf(dest, fmt, args);
        recomp_printf(dest);
    }
    va_end(args);
}

void _warning_noheader(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    if (get_log_level() >= LOG_WARNING)
    {
        vsprintf(dest, fmt, args);
        recomp_printf(dest);
    }
    va_end(args);
}

void _error_noheader(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    if (get_log_level() >= LOG_ERROR)
    {
        vsprintf(dest, fmt, args);
        recomp_printf(dest);
    }
    va_end(args);
}

void _critical_noheader(const char* fmt, ...)
{
    char dest[1100];
    va_list args;
    va_start(args, fmt);
    if (get_log_level() >= LOG_CRITICAL)
    {
        vsprintf(dest, fmt, args);
        recomp_printf(dest);
    }
    va_end(args);
}

void logger_init(Logger* logger)
{
    logger->dev = _dev;
    logger->debug = _debug;
    logger->info = _info;
    logger->warning = _warning;
    logger->error = _error;
    logger->critical = _critical;

    logger->noheader.dev = _dev_noheader;
    logger->noheader.debug = _debug_noheader;
    logger->noheader.info = _info_noheader;
    logger->noheader.warning = _warning_noheader;
    logger->noheader.error = _error_noheader;
    logger->noheader.critical = _critical_noheader;

    update_log_level();

    logger->is_initialized = true;
}

void set_log_level(LogLevel level)
{
    _set_log_level(level);
}

LogLevel get_log_level()
{
    return (LogLevel)_get_log_level();
}

// Only prints to dev
void print_bytes(void* addr, int n)
{
    if (get_log_level() > LOG_DEV) return;

    recomp_printf("Data starting from %p is:\n\n", addr);
    recomp_printf("\t\t00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n\n");

    uintptr_t addrInt = (uintptr_t) addr;
    recomp_printf("%08x\t", addrInt);

    for (int i = 0; i < n; i++)
    {
        if (i % 16 == 0 && i != 0) 
        {
            recomp_printf("\n%08x\t", addrInt + i);
        }
        
        recomp_printf("%02x ", *(unsigned char*)(addr + i));
    }
    recomp_printf("\n\n");
}

extern Logger logger;
RECOMP_HOOK("Play_Main") void refresh_log_level()
{
    update_log_level();
}

void update_log_level()
{
    LogLevel setLevel = recomp_get_config_u32("log_level");
    if (get_log_level() != setLevel)
    {
        set_log_level(setLevel);
        recomp_printf("[MUSIC RANDOMIZER] Set log level to ");
        switch(setLevel)
        {
            case 0: recomp_printf("Nothing"); break;
            case 1: recomp_printf("Critical"); break;
            case 2: recomp_printf("Error"); break;
            case 3: recomp_printf("Warning"); break;
            case 4: recomp_printf("Info"); break;
            case 5: recomp_printf("Debug"); break;
            case 6: recomp_printf("Dev"); break;
        }
        recomp_printf(".\n");
    }
}

// Assume s is 20 bytes already allocated
void sprintf_binary(char* s, u16 val)
{
    for (int i = 15; i >= 0; i--)
    {
        *s++ = (val & (1 << i)) ? '1' : '0';
        if (i % 4 == 0 && i != 0)
            *s++ = ' ';
    }
    *s++ = '\0';
}