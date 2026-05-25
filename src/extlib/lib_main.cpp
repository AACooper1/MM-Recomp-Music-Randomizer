#include <iostream>

#include "lib_recomp.hpp"

#include "database.h"
#include "seed.h"
#include "modtrackdefs.h"
#include "ootrs.hpp"

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

Log logger;

std::shared_ptr<Database> db;
std::shared_ptr<Seed> seed;

OoTAudioHandler* ootAudioHandler;
bool can_use_ootrs;

RECOMP_DLL_FUNC(prepare_database) {
    std::string modPath = RECOMP_ARG_STR(0);
    
    logger.dev << "Extlib-side logger OK!" << std::endl;

    fs::path dbPath = (fs::path)modPath;
    dbPath = dbPath.parent_path();
    dbPath /= "mod_data";

    try 
    {
        db = std::make_shared<Database>(dbPath);
        db->init();
    }
    catch (std::exception& e)
    {
        logger.error << e.what() << std::endl;
        RECOMP_RETURN(int, 1);
    }

    int rc = 0;
    
    try
    {
        rc = db->update_from_music_dir();
        rc = db->load_all_tracks();
        logger.dev << "Successfully loaded tracks!" << std::endl;
    }
    catch (std::exception& e)
    {
        logger.error << e.what() << std::endl;
        RECOMP_RETURN(int, -1);
    }

    RECOMP_RETURN(int, rc);
}

RECOMP_DLL_FUNC(check_seed_exists) 
{
    fs::path savePath = RECOMP_ARG_STR(0);
    savePath = savePath.replace_extension(".music.db");
    if (fs::exists(savePath))
    {
        RECOMP_RETURN(int, true);
    }
    else
    {
        RECOMP_RETURN(int, false);
    }
}

RECOMP_DLL_FUNC(prepare_seed)
{
    int randoSeed = RECOMP_ARG(int, 0);
    fs::path savePath = RECOMP_ARG_STR(1);
    bool use_custom = RECOMP_ARG(bool, 2);
    bool use_vanilla = RECOMP_ARG(bool, 3);

    savePath = savePath.replace_extension(".music.db");
    seed = std::make_shared<Seed>(randoSeed, db, savePath, use_custom, use_vanilla, can_use_ootrs);

    try
    {
        if (fs::exists(savePath))
        {
            seed->load_seed(savePath);
        }
        else
        {
            seed->randomize();
            logger.debug << "Successfully finished randomization!" << std::endl;
        }
        if (seed->songforceTracks.size())
        {
            seed->apply_songforce();
            logger.debug << "Applied songforce!" << std::endl;
        }
        seed->save_seed();
        logger.debug << "Saved seed!" << std::endl;
        seed->apply_songtest();
        logger.debug << "Applied songtest. Returning from prepare_seed..." << std::endl;
    }
    catch (std::exception& e)
    {
        logger.error << "Could not prepare seed: " << e.what() << std::endl;
        RECOMP_RETURN (int, false);
    }
    RECOMP_RETURN (int, true);
}

void copy_into_mod_ram(char* dst, char* src, size_t size)
{
    for (int i = 0; i < size; i++)
    {
        dst[i^3] = src[i];
    }

    return;
}

void prepare_vanilla_track(std::shared_ptr<Track> extlibTrack, cTrack* modTrack)
{
    modTrack->type = cTrackType(TrackType::VANILLA);
    for (int i = 0; i < 16; i++) { modTrack->formmask.states[i] = 0xFFFF; }
    modTrack->formmask.cumulativeStates = 0xFFFF;

    modTrack->seq.id = extlibTrack->id - 0x200;
}

void prepare_custom_track(std::shared_ptr<Track> extlibTrack, cTrack* modTrack)
{
    modTrack->type = cTrackType(extlibTrack->type);

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

    modTrack->numSounds = 0;

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
        modTrack->formmask.states[i] = extlibTrack->formmask.states[i ^ 1];
    }
    modTrack->formmask.pad[0] = extlibTrack->formmask.cumulativeStates;
}

RECOMP_DLL_FUNC(reroll_slot)
{
    int slotIdx = RECOMP_ARG(int, 0);

    seed->randomize_slot(slotIdx);
}

RECOMP_DLL_FUNC(read_oot_audiobin)
{
    fs::path ootAudioBinPath = db->get_db_dir() / "OOT.audiobin";
    if (fs::exists(ootAudioBinPath))
    {
        logger.info << "Found OoT audiobin, reading..." << std::endl;
        ootAudioHandler = new OoTAudioHandler(ootAudioBinPath);

        ootAudioHandler->just_testing_this_now();

        if (ootAudioHandler->successfully_parsed)
        {
            logger.info.disable_header();
            logger.info << "Success!" << std::endl;
            logger.info.enable_header();
            RECOMP_RETURN(bool, true);
        }
    }
    else
    {
        logger.info << "No OoT audiobin found, will not use OOTRS files." << std::endl;
    }
    RECOMP_RETURN(bool, false);
}

RECOMP_DLL_FUNC(get_oot_audiobin_headers)
{
    AudioTableHeader* soundTableHeader = RECOMP_ARG(AudioTableHeader*, 0);
    AudioTableHeader* bankTableHeader = RECOMP_ARG(AudioTableHeader*, 1);
    

    if (ootAudioHandler->successfully_parsed)
    {
        can_use_ootrs = true;
        std::memcpy(soundTableHeader, ootAudioHandler->ootFiles[AUDIOTABLE_HEADER].data(), 0x10);
        std::memcpy(bankTableHeader, ootAudioHandler->ootFiles[BANKTABLE_HEADER].data(), 0x10);
    }
}

RECOMP_DLL_FUNC(get_oot_audiobin_entries)
{
    AudioTableEntry* soundTableEntries = RECOMP_ARG(AudioTableEntry*, 0);
    AudioTableEntry* bankTableEntries = RECOMP_ARG(AudioTableEntry*, 1);

    if (ootAudioHandler->successfully_parsed)
    {
        std::memcpy(soundTableEntries, ootAudioHandler->ootFiles[AUDIOTABLE_HEADER].data() + 0x10, ootAudioHandler->ootFiles[AUDIOTABLE_HEADER].size() - 0x10);
        std::memcpy(bankTableEntries, ootAudioHandler->ootFiles[BANKTABLE_HEADER].data() + 0x10, ootAudioHandler->ootFiles[BANKTABLE_HEADER].size() - 0x10);
    }
}

RECOMP_DLL_FUNC(get_oot_bank_data)
{
    u8* dest = RECOMP_ARG(u8*, 0);
    int blob_index = RECOMP_ARG(int, 1);
    int size = RECOMP_ARG(int, 2);

    if (ootAudioHandler->successfully_parsed)
    {
        if (blob_index + size > ootAudioHandler->ootFiles[BANKTABLE].size())
        {
            logger.error << "Got OoT bank data offset outside blob size!! Track will be rerolled." << std::endl;
        }
        else
        {
            std::memcpy(dest, ootAudioHandler->ootFiles[BANKTABLE].data() + blob_index, size);
            RECOMP_RETURN(bool, true);
        }
    }
    RECOMP_RETURN(bool, false);
}


RECOMP_DLL_FUNC(get_oot_sound_data)
{
    u8* dest = RECOMP_ARG(u8*, 0);
    int blob_index = RECOMP_ARG(int, 1);
    int size = RECOMP_ARG(int, 2);

    if (ootAudioHandler->successfully_parsed)
    {
        if (blob_index + size > ootAudioHandler->ootFiles[AUDIOTABLE].size())
        {
            logger.error << "Got OoT sound data offset outside blob size!! Track will be rerolled." << std::endl;
        }
        else
        {
            std::memcpy(dest, ootAudioHandler->ootFiles[AUDIOTABLE].data() + blob_index, size);
            RECOMP_RETURN(bool, true);
        }
    }
    RECOMP_RETURN(bool, false);
}

RECOMP_DLL_FUNC(disable_ootrs)
{
    can_use_ootrs = false;
}

RECOMP_DLL_FUNC(fetch_randomized_track)
{
    int slotIdx = RECOMP_ARG(int, 0);
    cTrack* modTrack = RECOMP_ARG(cTrack*, 1);
    cTrack tempTrack;

    std::shared_ptr<Track> extlibTrack = seed->randomized[slotIdx];

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
    modTrack->slotIdx = slotIdx;

    if (extlibTrack->type != TrackType::VANILLA)
    {
        prepare_custom_track(extlibTrack, modTrack);
    }
    else
    {
        prepare_vanilla_track(extlibTrack, modTrack);
    }
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