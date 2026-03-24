#include "main.h"

RECOMP_CALLBACK("*", recomp_on_init) void init_loggers()
{
    logger_init(&logger);
}

RECOMP_HOOK("Play_Update") void extract_gamestate(PlayState* this)
{
    playCtx = this;
    return;
}

RECOMP_HOOK_RETURN("Play_Update") void after_play_update()
{
    update_music_state(playCtx);
    apply_mask();
}

void music_rando_update_db()
{
    update_log_level();
    logger.debug("%s", "Mod-side logger OK!\n");
    logger.noheader.debug("%s", "Mod-side no-header logger OK!\n");

    unsigned char* modPath = recomp_get_mod_folder_path();

    prepare_database(modPath);
    logger.noheader.debug("Finished prepare_database!\n");
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

RECOMP_HOOK("Setup_InitImpl") void launch_on_rando_connect()
{
    DependencyStatus found_rando = recomp_is_dependency_met("mm_recomp_rando");
    if (found_rando != DEPENDENCY_STATUS_FOUND)
    {
        return;
    }
    recomp_printf("Connected to rando. Starting music randomization...\n");
    music_rando_update_db();
}

RECOMP_CALLBACK(".", music_rando_begin_randomization) void music_rando_ready_seed()
{
    unsigned char* savePath = recomp_get_save_file_path();
    logger.noheader.dev("Save path: %s\n", savePath);

    int randoSeed;
    randoSeed = get_current_time();

    if (!prepare_seed(randoSeed, savePath, true, true))
    {
        logger.critical("Could not prepare seed, aborting music rando!\n");
        recomp_free(savePath);
        return;
    }
    recomp_free(savePath);
    logger.debug("Prepared seed!\n");

    prepare_tracks();
    logger.debug("Prepared tracks!\n");
    music_rando_seed_prepared(randomized);

    replace_tracks();

    randomization_complete = true;
    music_rando_randomization_complete(randomized);

    logger.info("");
    for (int i = 2; i < NUM_SONG_SLOTS; i++)
    {
        if (should_skip_song_title_display[i]) continue;
        logger.noheader.info("%s --> %s!", randomized[i].slotName, randomized[i].name);
        logger.noheader.dev("(%x --> %x)", randomized[i].slotIdx, randomized[i].seq.id);
        logger.noheader.info("\n");
    }
}

RECOMP_CALLBACK(".", music_rando_randomization_complete) void milk_bar_and_final_hours_fix()
{
    gSequenceFontTable[NA_BGM_MILK_BAR_DUPLICATE] = (u8)(randomized[NA_BGM_MILK_BAR].bankNo);
    Lib_MemCpy(randomized[NA_BGM_MILK_BAR_DUPLICATE].name, randomized[NA_BGM_MILK_BAR].name, 256);
    Lib_MemCpy(&(randomized[NA_BGM_MILK_BAR_DUPLICATE].formmask), &(randomized[NA_BGM_MILK_BAR].formmask), sizeof(cFormMask));

    gSequenceFontTable[NA_BGM_MAJORAS_LAIR] = (u8)(randomized[NA_BGM_FINAL_HOURS].bankNo);
    Lib_MemCpy(randomized[NA_BGM_MAJORAS_LAIR].name, randomized[NA_BGM_FINAL_HOURS].name, 256);
    Lib_MemCpy(&(randomized[NA_BGM_MAJORAS_LAIR].formmask), &(randomized[NA_BGM_FINAL_HOURS].formmask), sizeof(cFormMask));
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
            for (int i = 0; i < 16; i++) { randomized[i].formmask.states[i] = 0xFFFF; }
            randomized[i].formmask.cumulativeStates = 0xFFFF;

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
        logger.noheader.dev("Track %s formmask 0: %x\n", track->name, track->formmask.states[0]);
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

RECOMP_IMPORT("mm_bens_remastered_soundtrack", u8 BensSoundtrack_GetOriginalBankIdx(int seqId));
void replace_vanilla(int i)
{
    if (i == NA_BGM_FROG_SONG) return;

    AudioTableEntry* origTrack = &origTableCopy[randomized[i].seq.id];
    AudioApi_ReplaceSequence(i, origTrack);
    AudioApi_ReplaceSequenceFont(i, 0, randomized[i].bankNo);
    if (recomp_is_dependency_met("mm_bens_remastered_soundtrack") == DEPENDENCY_STATUS_FOUND)
    {
        u8 streamedBankNo = BensSoundtrack_GetOriginalBankIdx(randomized[i].seq.id);
        if (streamedBankNo) { AudioApi_ReplaceSequenceFont(i, 0, streamedBankNo); }
    }
}

extern u32 AudioLoad_GetRealTableIndex(s32 tableType, u32 id);
extern void* AudioLoad_SyncLoad(s32 tableType, u32 id, s32* didAllocate);

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

PlayState* gPlayState;
RECOMP_HOOK("Environment_PlaySceneSequence") void save_playstate(PlayState* play)
{
    gPlayState = play;
}

RECOMP_PATCH void Scene_CommandSoundSettings(PlayState* play, SceneCmd* cmd) {
    u8 ambienceId;

    ambienceId = cmd->soundSettings.ambienceId;
    if(recomp_get_config_u32("enable_night_bgm"))
    {
        // DAWN OF... checks. Day 2 & 3 occur at 16384 (6 AM), Day 1 occurs at 16383 (6 AM - 1)
        if (ambienceId == 0 && !(gSaveContext.save.time == CLOCK_TIME(6,0) || gSaveContext.save.time == CLOCK_TIME(18,0) || gSaveContext.save.time == 16383))
        {
            ambienceId = AMBIENCE_ID_13;
        }
    }

    play->sceneSequences.seqId = cmd->soundSettings.seqId;
    play->sceneSequences.ambienceId = ambienceId;

    if (gSaveContext.seqId == (u8)NA_BGM_DISABLED ||
        AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) == NA_BGM_FINAL_HOURS) {
        Audio_SetSpec(cmd->soundSettings.specId);
    }
}

RECOMP_IMPORT("mm_bens_remastered_soundtrack", void BensSoundtrack_SetDisableChannelSwitching(int playerIndex, bool shouldDisable));
RECOMP_HOOK("AudioLoad_SyncInitSeqPlayer") void bens_soundtrack_disable_switching(s32 playerIndex, s32 seqId, s32 arg2)
{
    if (recomp_is_dependency_met("mm_bens_remastered_soundtrack") != DEPENDENCY_STATUS_FOUND) return;
    if (randomized[seqId].type == VANILLA)
    {
        BensSoundtrack_SetDisableChannelSwitching(playerIndex, false);
    }
    else
    {
        BensSoundtrack_SetDisableChannelSwitching(playerIndex, true);
    }
}