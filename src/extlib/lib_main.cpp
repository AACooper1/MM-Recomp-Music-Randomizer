#include <iostream>

#include "lib_recomp.hpp"

#include "database.h"

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

Database *db = nullptr;
Log logger;

RECOMP_DLL_FUNC(update_database) {
    logger.set_log_level(LogLevel::LOG_DEV);
    logger.dev << "If you're seeing this, the extlib's logger works." << std::endl;

    std::string modPath = RECOMP_ARG_STR(0);

    fs::path dbPath = (fs::path)modPath;
    dbPath = dbPath.parent_path();
    dbPath /= "mod_data";

    try 
    {
        db = Database::get_db(dbPath);
    }
    catch (std::exception e)
    {
        logger.error << e.what() << std::endl;
        RECOMP_RETURN(int, 1);
    }

    int rc = 0;
    
    try
    {
        rc = db->update_from_music_dir();
    }
    catch (std::exception e)
    {
        logger.error << e.what() << std::endl;
        RECOMP_RETURN(int, -1);
    }

    RECOMP_RETURN(int, rc);
}

RECOMP_DLL_FUNC(_log)
{
    std::string msg = RECOMP_ARG_STR(0);
    LogLevel level = (LogLevel)RECOMP_ARG(int, 1);
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