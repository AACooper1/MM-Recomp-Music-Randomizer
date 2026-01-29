#include <iostream>

#include "lib_recomp.hpp"

#include "database.h"

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

std::shared_ptr<Database> db;
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
        db = std::make_shared<Database>(dbPath);
        db->init();
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
        rc = db->load_all_songs();
    }
    catch (std::exception& e)
    {
        logger.error << e.what() << std::endl;
        RECOMP_RETURN(int, -1);
    }

    std::shared_ptr<Track> test = db->tables->track->entries.at(2);
    logger.dev << "Name of first track: " << test->name << std::endl
               << "Formmask begins with: " << test->formmask.states[0] << std::endl;

    std::shared_ptr<Sequence> testSeq = db->tables->seq->entries.at(2);
    logger.dev << "ID of first seq: " << testSeq->databaseIndex << std::endl
               << "Data begins with: " << std::hex << (short)(*testSeq->data)[0] << std::hex << (int)(*testSeq->data)[1]  << std::endl;

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