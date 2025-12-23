#include <iostream>
#include <sstream>
#include <format>
#include <chrono>
#include <string>

class Log;

enum class LogLevel
{
    LOG_NOTHING,
    LOG_CRITICAL,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG,
    LOG_DEV
};

class Logger
{
    public:
        template<typename T>
        std::ostream& operator << (const T& text)
        {
            stream();
            dest << text;
            return dest;
        }

        template<typename ...P>
        void operator () (std::format_string<P...> format, P &&... params)
        {
            this->call(format, std::forward<P>(params)...);
        }
    
        private:
            template<typename ...P>
            void call(std::format_string<P...>, P &&... params);
            
            void stream();

            void printHeader();

            Logger(LogLevel level, Log* parent) : level(level), dest(std::cout), parent(parent) { }

            friend class Log;
            Log* parent;
            LogLevel level;
            std::ostream& dest;                 // short for Destember Holiday

            const std::string levels[7] = {"", "CRITICAL", "ERROR", "WARNING", "INFO", "DEBUG", "DEV"};
};

class Log
{
public:
    Log()
    : critical (LogLevel::LOG_CRITICAL, this),
      error    (LogLevel::LOG_ERROR,    this),
      warning  (LogLevel::LOG_WARNING,  this),
      info     (LogLevel::LOG_INFO,     this),
      debug    (LogLevel::LOG_DEBUG,    this),
      dev      (LogLevel::LOG_DEV,      this)
    {}

    Logger critical;
    Logger error;
    Logger warning;
    Logger info;
    Logger debug;
    Logger dev;

    void set_log_level(LogLevel level) { gLogLevel = level; }
    LogLevel get_log_level() const { return gLogLevel; }

private:
    friend class Logger;
    LogLevel gLogLevel = LogLevel::LOG_INFO;
    
    std::chrono::system_clock::time_point time = std::chrono::system_clock::now();

    std::string parentModName = "MUSIC RANDOMIZER";
    std::string header;
};


template<typename ...P>
void Logger::call(std::format_string<P...> format, P &&... params)
{
    this->dest.clear();
    if (parent->get_log_level() > level)
    {
        std::string msg = std::format(format, std::forward<P>(params)...);
        printHeader();
        dest << msg;
    }
}