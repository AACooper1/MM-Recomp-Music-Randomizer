#include "logging.hpp"

#include "lib_recomp.hpp"

void Logger::stream()
{
    dest.clear();
    if (parent->get_log_level() >= level)
    {
        printHeader();
    }
    else
    {
        dest.setstate(std::ios::failbit);
    }

    return;
}

void Logger::printHeader() 
{
    dest << std::endl << 
            "[" << parent->parentModName << "] " << 
            "[" << parent->time << "] " <<
            "[" << levels[(int)level] << "]"
        << std::endl;
}