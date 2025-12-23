#include "lib_recomp.hpp"
#include "util.h"

#include <chrono>
#include <ctime>

RECOMP_DLL_FUNC(get_current_time)
{
    RECOMP_RETURN(u32, (u32)(time(nullptr) & 0xFFFFFF));
}

RECOMP_DLL_FUNC(fucking_use_stoi)
{
    std::string whatever = RECOMP_ARG_STR(0);

    RECOMP_RETURN(int, (int)std::stoll(whatever) % INT32_MAX);
}