#include "lib_recomp.hpp"
#include "util.h"

#include <ctime>

RECOMP_DLL_FUNC(get_current_time)
{
    RECOMP_RETURN(u32, (u32)(time(nullptr) & 0xFFFFFF));
}


void mmrs_util::set_log_level(log_level_t level)
{
    gLogLevel = level;
}

std::ostream* mmrs_util::_log(log_level_t level)
{
    std::cout.clear();
    if (gLogLevel < level)
    {
        std::cout.setstate(std::ios::failbit);
    }

    return &std::cout;
}

std::ostream&  mmrs_util::critical() {return *mmrs_util::_log(LOG_CRITICAL);}
std::ostream&  mmrs_util::error() {return *mmrs_util::_log(LOG_ERROR);}
std::ostream&  mmrs_util::warning() {return *mmrs_util::_log(LOG_WARNING);}
std::ostream&  mmrs_util::info() {return *mmrs_util::_log(LOG_INFO);}
std::ostream&  mmrs_util::debug() {return *mmrs_util::_log(LOG_DEBUG);}