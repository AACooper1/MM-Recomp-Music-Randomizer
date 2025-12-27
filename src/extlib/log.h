#pragma once

class Logger;

class Log
{
public:
    void set_log_level(LogLevel level) { gLogLevel = level; }
    LogLevel get_log_level() { return gLogLevel; }

    Logger critical = Logger(LogLevel::LOG_CRITICAL, this);
    Logger error = Logger(LogLevel::LOG_ERROR, this);
    Logger warning = Logger(LogLevel::LOG_WARNING, this);
    Logger info = Logger(LogLevel::LOG_INFO, this);
    Logger debug = Logger(LogLevel::LOG_DEBUG, this);
    Logger dev = Logger(LogLevel::LOG_DEV, this);

private:
    friend class Logger;
    LogLevel gLogLevel = LogLevel::LOG_INFO;
    
    std::chrono::system_clock::time_point time = std::chrono::system_clock::now();

    std::string parentModName = "MUSIC RANDOMIZER";
    std::string header;
};


