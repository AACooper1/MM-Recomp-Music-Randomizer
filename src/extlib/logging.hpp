#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <format>
#include <chrono>
#include <string>
#include <filesystem>

class Log;
namespace fs = std::filesystem;

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

extern Log logger;

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
        void set_outfile(std::string path) 
        { 
            if (file.is_open())
            {
                file.close();
            }
            file.open(path);
            dest.rdbuf(file.rdbuf());
            return;
        }
        void set_out_console()
        {
            dest.rdbuf(cout);
        }
        std::ostream& get_dest()
        {
            return dest;
        }

        template<typename ...P>
        void operator () (std::format_string<P...> format, P &&... params)
        {
            this->call(format, std::forward<P>(params)...);
        }

        void disable_header() { shouldPrintHeader = false; }
        void enable_header() { shouldPrintHeader = true; }
    
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
            std::ofstream file;
            std::streambuf* cout = std::cout.rdbuf();

            bool shouldPrintHeader = true;

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
    void set_outfile()
    {
        for (int i = 0; i < 6; i++)
        {
            loggers[i]->set_outfile(filePath);
        }
    }
    void set_out_console()
    {
        for (int i = 0; i < 6; i++)
        {
            loggers[i]->set_out_console();
        }
    }

    Logger critical;
    Logger error;
    Logger warning;
    Logger info;
    Logger debug;
    Logger dev;

    Logger* loggers[6] = {&critical, &error, &warning, &info, &debug, &dev};
    std::string filePath;
    bool dest_is_file = false;

    void set_log_level(LogLevel level) { gLogLevel = level; }
    LogLevel get_log_level() const { return gLogLevel; }

    std::chrono::system_clock::time_point get_time()
    {
        return std::chrono::system_clock::now();
    }

private:
    friend class Logger;
    LogLevel gLogLevel = LogLevel::LOG_INFO;

    std::string parentModName = "MUSIC RANDOMIZER";
    std::string header;
};


template<typename ...P>
void Logger::call(std::format_string<P...> format, P &&... params)
{
    this->dest.clear();
    if (parent->get_log_level() >= level)
    {
        std::string msg = std::format(format, std::forward<P>(params)...);
        if (shouldPrintHeader) printHeader();
        dest << msg;
    }
}

#endif