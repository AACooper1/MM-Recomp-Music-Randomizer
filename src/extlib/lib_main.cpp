#include <iostream>

#include "lib_recomp.hpp"

#include "database.h"
#include "seed.h"
#include "modtrackdefs.h"

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

std::shared_ptr<Database> db;
Log logger;
std::shared_ptr<Seed> seed;

RECOMP_DLL_FUNC(prepare_database) {
    std::string modPath = RECOMP_ARG_STR(0);
    
    logger.set_log_level(LogLevel::LOG_DEV);
    logger.dev << "Extlib-side logger OK!" << std::endl;

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
        rc = db->load_all_tracks();
    }
    catch (std::exception& e)
    {
        logger.error << e.what() << std::endl;
        RECOMP_RETURN(int, -1);
    }

    RECOMP_RETURN(int, rc);
}

RECOMP_DLL_FUNC(prepare_seed)
{
    int randoSeed = RECOMP_ARG(int, 0);
    fs::path savePath = RECOMP_ARG_STR(1);
    bool use_custom = RECOMP_ARG(bool, 2);
    bool use_vanilla = RECOMP_ARG(bool, 3);

    savePath = savePath.replace_extension(".music.db");
    seed = std::make_shared<Seed>(randoSeed, db, savePath, use_custom, use_vanilla);

    if (fs::exists(savePath))
    {
        seed->load_seed(savePath);
    }
    else
    {
        seed->randomize();
        seed->save_seed();
    }
}

void copy_into_mod_ram(char* dst, char* src, size_t size)
{
    for (int i = 0; i < size; i++)
    {
        dst[i^3] = src[i];
    }

    return;
}

RECOMP_DLL_FUNC(fetch_randomized_track)
{
    int slotIdx = RECOMP_ARG(int, 0);
    cTrack* modTrack = RECOMP_ARG(cTrack*, 1);
    cTrack tempTrack;

    std::shared_ptr<Track> extlibTrack = seed->randomized[slotIdx];
    
    modTrack->type = cTrackType(extlibTrack->type);

    for(int i = 0; i < extlibTrack->name.length() && i < 256; i++)
    {
        modTrack->name[i ^ 3] = extlibTrack->name[i];
    }

    std::string slotName = seed->get_slot_name(SongSlotID(slotIdx));

    for(int i = 0; i < slotName.length() && i < 256; i++)
    {
        modTrack->slotName[i ^ 3] = slotName[i];
    }

    modTrack->bankNo = extlibTrack->bankNo;
    modTrack->tableIdx = slotIdx;

    if (extlibTrack->sequence)
    {
        modTrack->hasSeq = true;
        modTrack->seq.size = extlibTrack->sequence->size;
        modTrack->seq.id = extlibTrack->seqId;
    }
    else
    {
        modTrack->hasSeq = false;
    }
    
    if (extlibTrack->bank)
    {
        modTrack->hasBank = true;
        
        for (int i = 0; i < 0x08; i++)
        {
            modTrack->bank.header[i^3] = extlibTrack->bank->header->data()[i];
        }

        modTrack->bank.size = extlibTrack->bank->size;
        modTrack->bank.id = extlibTrack->bankId;
    }
    else
    {
        modTrack->hasBank = false;
    }

    modTrack->numSounds = extlibTrack->soundIds.size();

    for (int i = 0; i < extlibTrack->soundIds.size(); i++)
    {
        if (extlibTrack->soundIds.size() > 64)
        {
            logger.error << "Track " << extlibTrack->name << " had more than 64 sounds, which is honestly impressive but too many for music rando to handle. Sorry." << std::endl;
            return;
        }
        modTrack->numSounds++;
        modTrack->sounds[i].id = extlibTrack->soundIds[i];
        modTrack->sounds[i].size = extlibTrack->sounds[i]->size;
        modTrack->sounds[i].sampleAddr = extlibTrack->sounds[i]->sampleAddr;
    }
    
    for (int i = 0; i < 16; i++)
    {
        modTrack->formmask.states[i] = extlibTrack->formmask.states[i];
    }
    modTrack->formmask.cumulativeStates = extlibTrack->formmask.cumulativeStates;
}

RECOMP_DLL_FUNC(fetch_seq)
{
    int id = RECOMP_ARG(int, 0);
    char* dst = RECOMP_ARG(char*, 1);
    int size = RECOMP_ARG(int, 2);

    char* seq = db->tables->seq->entries[id]->data->data();

    copy_into_mod_ram(dst, seq, size);
}

RECOMP_DLL_FUNC(fetch_bank)
{
    int id = RECOMP_ARG(int, 0);
    char* dst = RECOMP_ARG(char*, 1);
    int size = RECOMP_ARG(int, 2);

    char* bank = db->tables->bank->entries[id]->data->data();

    copy_into_mod_ram(dst, bank, size);
}

RECOMP_DLL_FUNC(fetch_sound)
{
    int id = RECOMP_ARG(int, 0);
    char* dst = RECOMP_ARG(char*, 1);
    int size = RECOMP_ARG(int, 2);

    char* sound = db->tables->sound->entries[id]->data->data();

    copy_into_mod_ram(dst, sound, size);
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

RECOMP_DLL_FUNC(get_current_time)
{
    RECOMP_RETURN(u32, (u32)(time(nullptr) & 0xFFFFFF));
}