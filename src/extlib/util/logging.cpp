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
    int level = RECOMP_ARG(int, 1) - 1;
    bool noheader = RECOMP_ARG(bool, 2);
    if (logger.get_log_level() > LogLevel::LOG_DEV) logger.set_log_level(LogLevel::LOG_DEV);
    
    if (noheader) logger.loggers[level]->disable_header();
    else logger.loggers[level]->enable_header();
    
    *logger.loggers[level] << msg;
    logger.loggers[level]->get_dest().flush(); // lol antipattern ass
    
    logger.loggers[level]->enable_header();
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

RECOMP_DLL_FUNC(_set_log_console)
{
    logger.set_out_console();
    logger.dest_is_file = false;
}

RECOMP_DLL_FUNC(_set_log_file)
{
    std::string path = RECOMP_ARG_STR(0);
    fs::path filePath = (fs::path)path;
    filePath = filePath.parent_path();
    filePath /= "mod_data";
    
    logger.filePath = (filePath / "musicRando_log.txt").string();
    
    logger.set_outfile();
    logger.dest_is_file = true;
}

RECOMP_DLL_FUNC(_get_log_dest)
{
    RECOMP_RETURN(int, logger.dest_is_file);
}