#ifndef LOGGING_H
#define LOGGING_H

// MM Recomp Mod Requirements
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "libc64/sprintf.h"

typedef enum LogLevel_t
{
    LOG_NOTHING,
    LOG_CRITICAL,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_DEV
} LogLevel;

typedef struct Logger_noheader_t
{
    void (*dev)(const char* fmt, ...);
    void (*debug)(const char* fmt, ...);
    void (*info)(const char* fmt, ...);
    void (*warning)(const char* fmt, ...);
    void (*error)(const char* fmt, ...);
    void (*critical)(const char* fmt, ...);
} Logger_noheader;

typedef struct Logger_t
{
    bool is_initialized;
    void (*dev)(const char* fmt, ...);
    void (*debug)(const char* fmt, ...);
    void (*info)(const char* fmt, ...);
    void (*warning)(const char* fmt, ...);
    void (*error)(const char* fmt, ...);
    void (*critical)(const char* fmt, ...);

    Logger_noheader noheader;
} Logger;

void sprintf_binary(char* s, u16 val);

void logger_init(Logger* logger);

void set_log_level(LogLevel level);
LogLevel get_log_level();

void update_log_level();
void update_log_dest();

void print_bytes(Logger* logger, void* addr, int n);

#endif