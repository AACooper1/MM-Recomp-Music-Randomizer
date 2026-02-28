#include "main.h"

void music_rando_update_db()
{
    logger_init(&logger);
    set_log_level(LOG_DEV);
    logger.debug("%s", "Mod-side logger OK!\n");
    logger.noheader.debug("%s", "Mod-side no-header logger OK!\n");

    unsigned char* modPath = recomp_get_mod_folder_path();

    prepare_database(modPath);
    logger.debug("Finished prepare_database!\n");
    music_rando_begin_randomization();
}

RECOMP_HOOK_RETURN("ConsoleLogo_Init") void begin_if_no_rando()
{
    DependencyStatus found_rando = recomp_is_dependency_met("mm_recomp_rando");
    if (found_rando != DEPENDENCY_STATUS_FOUND)
    {
        logger.debug("Rando not found: %i\n", found_rando);
        music_rando_update_db();
    }
}

RECOMP_CALLBACK("mm_recomp_rando", rando_on_connect) void launch_on_rando_connect()
{
    recomp_printf("Connected to rando. Starting music randomization...\n");
    music_rando_update_db();
}

RECOMP_CALLBACK(".", music_rando_begin_randomization) void music_rando_ready_seed()
{
    unsigned char* savePath = recomp_get_save_file_path();
    logger.noheader.dev("Save path: %s\n", savePath);

    int randoSeed;
    randoSeed = get_current_time();

    prepare_seed(randoSeed, savePath, true, true);
    recomp_free(savePath);
    logger.debug("Prepared seed!\n");

    prepare_tracks();
    logger.debug("Prepared tracks!\n");

    replace_tracks();

    logger.info("");
    for (int i = 2; i < NUM_SONG_SLOTS; i++)
    {
        logger.noheader.info("Randomized %s to %s!", randomized[i].slotName, randomized[i].name);
        logger.noheader.dev("(%x --> %x)\n", randomized[i].slotIdx, randomized[i].seq.id);
    }
}

AudioTableEntry* origTableCopy;

void prepare_tracks()
{
    origTableCopy = recomp_alloc(sizeof(AudioTableEntry) * gAudioCtx.sequenceTable->header.numEntries);
    Lib_MemCpy(origTableCopy, gAudioCtx.sequenceTable->entries, sizeof(AudioTableEntry) * gAudioCtx.sequenceTable->header.numEntries);
    // Start at 2, because we want to skip the SFX and Ambience entries.
    for (int i = 2; i < NUM_SONG_SLOTS; i++)
    {
        fetch_randomized_track(i, &randomized[i]);
        if (randomized[i].type == VANILLA)
        {
            randomized[i].seq.id += 0x100;
        }
        else
        {
            populate_custom_track(&randomized[i]);
        }
    }
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
        logger.warning("Track %s does not have a sequence!\n", track->name);
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
    cTrack* track = &randomized[i];
    AudioTableEntry* mySeq = create_seq_entry_from_track(track);
    AudioApi_ReplaceSequence(i, mySeq);

    if (track->hasBank)
    {
        track->bankNo = create_bank_entry_from_track(track);
    }
    AudioApi_ReplaceSequenceFont(i, 0, track->bankNo);
}

void replace_vanilla(int i)
{
    AudioTableEntry* mySeq = create_seq_entry_from_track(&randomized[i]);

    AudioApi_ReplaceSequence(i, &origTableCopy[randomized[i].seq.id]);
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