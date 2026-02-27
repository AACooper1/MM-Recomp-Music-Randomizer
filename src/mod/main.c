#include "main.h"

RECOMP_HOOK_RETURN("ConsoleLogo_Init") void music_rando_update_db()
{
    logger_init(&logger);
    set_log_level(LOG_DEV);
    logger.debug("%s", "Mod-side logger OK!\n");
    logger.noheader.debug("%s", "Mod-side no-header logger OK!\n");

    unsigned char* modPath = recomp_get_mod_folder_path();

    prepare_database(modPath);
    music_rando_db_updated();
}

RECOMP_CALLBACK(".", music_rando_db_updated) void music_rando_ready_seed()
{
    unsigned char* savePath = recomp_get_save_file_path();

    int randoSeed;

    if (recomp_is_dependency_met("mm_recomp_rando") == DEPENDENCY_STATUS_FOUND)
    {
        randoSeed = rando_get_random_seed_external();
    }
    else
    {
        randoSeed = get_current_time();
    }

    prepare_seed(randoSeed, savePath, true, true);

    prepare_tracks();

    replace_tracks();

    logger.info("");
    for (int i = 2; i < NUM_SONG_SLOTS; i++)
    {
        logger.noheader.info("Randomized %s to %s!", randomized[i].slotName, randomized[i].name);
        logger.noheader.dev("(%x --> %x)\n", randomized[i].slotIdx, randomized[i].seq.id);
    }
}

void prepare_tracks()
{
    // Start at 2, because we want to skip the SFX and Ambience entries.
    for (int i = 2; i < NUM_SONG_SLOTS; i++)
    {
        fetch_randomized_track(i, &randomized[i]);
        if (randomized[i].type == VANILLA)
        {
            populate_vanilla_track(&randomized[i]);
        }
        else
        {
            populate_custom_track(&randomized[i]);
        }
    }
}

void populate_vanilla_track(cTrack* track)
{
    track->seq.id += 0x100;
    track->seq.data = (char*)gAudioCtx.sequenceTable->entries[track->seq.id].romAddr;
    logger.dev("Populated %s. ID is %i, addr is %p.\n", track->name, track->seq.id, track->seq.data);
}

void populate_custom_track(cTrack* track)
{
    if (track->hasSeq)
    {
        track->seq.data = recomp_alloc(track->seq.size);
        fetch_seq(track->seq.id, track->seq.data, track->seq.size);
    }
    else
    {
        logger.dev("Track %s does not have a sequence!\n", track->name);
    }
    if (track->hasBank)
    {
        track->bank.data = recomp_alloc(track->bank.size);
        fetch_bank(track->bank.id, track->bank.data, track->bank.size);
    }
    for (int s = 0; s < track->numSounds; s++)
    {
        track->sounds[s].data = recomp_alloc(track->sounds[s].size);
        fetch_sound(track->sounds[s].id, track->sounds[s].data, track->sounds[s].size);
    }   
}

AudioTableEntry* create_seq_entry_from_track(cTrack* track)
{
    logger.dev("Creating entry for %s (id %x)...\n", track->name, track->seq.id);
    AudioTableEntry* mySeq = recomp_alloc(sizeof(AudioTableEntry));

    mySeq->romAddr = (uintptr_t) track->seq.data;
    mySeq->size = track->seq.size;
    mySeq->medium = MEDIUM_CART;
    mySeq->cachePolicy = CACHE_EITHER;
    mySeq->shortData1 = track->slotIdx;
    mySeq->shortData2 = 0;
    mySeq->shortData3 = 0;

    return mySeq;
}

void link_custom_sound(cTrack* track, int soundIdx, u32* bank)
{
    for (int s = 0; s < track->numSounds; s++)
    {
        u32 sampleAddr = track->sounds[s].sampleAddr;
        for (int i = 0; i < track->bank.size / sizeof(u32); i++)
        {
            // logger.noheader.dev("\t@%04x: %02x %02x %02x %02x\n", i, bank[i], bank[i + 1], bank[i + 2], bank[i + 3]);
            if (bank[i] == sampleAddr)
            {
                bank[i] = (u32)track->sounds[s].data;
                break;
            }
        }
    }
}

s32 create_bank_entry_from_track(cTrack* track)
{
    for (int i = 0; i < track->numSounds; i++)
    {
        link_custom_sound(track, i, track->bank.data);
    }

    s32 bankNo = AudioApi_ImportVanillaSoundFont(
        (uintptr_t*)&(track->bank.data[0]),         // Addr
        track->bank.header[2],                      // sampleBank0
        track->bank.header[3],                      // sampleBank1
        track->bank.header[4],                      // numInstruments
        track->bank.header[5],                      // numDrums
        track->bank.header[6]                       // numSfx
    );
    
    return bankNo;
}

void replace_custom(int i)
{
    logger.dev("Replacing track %s\n", randomized[i].name);
    logger.noheader.dev("(type is %i)", randomized[i].type);
    cTrack* track = &randomized[i];
    AudioTableEntry* mySeq = create_seq_entry_from_track(track);
    logger.noheader.dev("\tCreated seq\n");
    AudioApi_ReplaceSequence(i, mySeq);

    if (track->hasBank)
    {
        track->bankNo = create_bank_entry_from_track(track);
        logger.noheader.dev("\tCreated bank\n");
    }
    else
    {
        logger.noheader.dev("\tUses vanilla bank\n");
    }
    AudioApi_ReplaceSequenceFont(i, 0, track->bankNo);
    logger.noheader.dev("Replaced sequence font!");
}

void replace_vanilla(int i)
{
    AudioTableEntry* mySeq = create_seq_entry_from_track(&randomized[i]);

    AudioApi_ReplaceSequence(i, mySeq);
    AudioApi_ReplaceSequenceFont(i, 0, randomized[i].bankNo);
}

void replace_tracks()
{
    for (int i = 2; i < 0x7F; i++)
    {
        if (randomized[i].type != VANILLA)
        {
            replace_custom(randomized[i].slotIdx);
        }
        else
        {
            replace_vanilla(randomized[i].slotIdx);
        }
    }
}