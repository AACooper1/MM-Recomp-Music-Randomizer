#include "logging.hpp"

#include "lib_recomp.hpp"

void Logger::stream()
{
    dest.clear();
    if (parent->get_log_level() >= level)
    {
        if (shouldPrintHeader) printHeader();
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
            "[" << parent->get_time() << "] " <<
            "[" << levels[(int)level] << "]"
        << std::endl;
}


RECOMP_DLL_FUNC(_log)
{
    std::string msg = RECOMP_ARG_STR(0);
    LogLevel level = (LogLevel)RECOMP_ARG(int, 1);
    if (logger.get_log_level() > LogLevel::LOG_DEV) logger.set_log_level(LogLevel::LOG_DEV);
    switch (level)
    {
        case LogLevel::LOG_DEV: logger.dev << msg; break;
        case LogLevel::LOG_DEBUG: logger.debug << msg; break;
        case LogLevel::LOG_INFO: logger.info << msg; break;
        case LogLevel::LOG_WARNING: logger.warning << msg; break;
        case LogLevel::LOG_ERROR: logger.error << msg; break;
        case LogLevel::LOG_CRITICAL: logger.critical << msg; break;
        default: break;
    }
}

RECOMP_DLL_FUNC(_set_log_level)
{
    LogLevel level = LogLevel(RECOMP_ARG(int, 0));
    logger.set_log_level(level);
}

RECOMP_DLL_FUNC(_get_log_level)
{
    RECOMP_RETURN(int, logger.get_log_level());
}

