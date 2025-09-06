// C Libraries
#include "stdint.h"
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"

// MM Recomp Mod Requirements
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

// Required Mods
#include "audio_api/all.h"

// Project Files
#include "read_mmrs.h"
#include "mod_util.h"



#ifndef SEQ_FLAG_FANFARE
    #define SEQ_FLAG_FANFARE (1 << 1)
#endif

RECOMP_IMPORT(".", int set_log_level(int level));
RECOMP_IMPORT(".", bool sql_init(const char *dbPath));
RECOMP_IMPORT(".", int read_mmrs_files());
RECOMP_IMPORT(".", int count_zsound(int mmrsId));
RECOMP_IMPORT(".", bool load_mmrs_table(MMRS* allMmrs));
RECOMP_IMPORT(".", bool load_zseq(Zseq* zseqAddr, int zseqId));
RECOMP_IMPORT(".", bool load_zbank(Zbank* zbankAddr, int zbankId));
RECOMP_IMPORT(".", bool load_zsounds(Zsound* zsoundAddr, int zsoundId));
RECOMP_IMPORT(".", void zsound_key_add(u32 key, u32 value));
RECOMP_IMPORT(".", void zsound_key_remove(u32 key));
RECOMP_IMPORT(".", u32 zsound_key_lookup(u32 key));
RECOMP_IMPORT(".", bool sql_teardown());

RECOMP_DECLARE_EVENT(music_rando_begin());
RECOMP_DECLARE_EVENT(mmrs_reader_done(MMRS *allMmrs, int numMmrs));

extern Vector* songNames;
extern int* randomizedIds;

MMRS *allMmrs;
int numMmrs;

Vector* zsoundTable;

int logLevel;

/*
    Initialize the music randomizer during N64 logo screen.
    This ensures that it loads after the rando has changed the save path.
    Will be replaced once rando update with declared event is out.
*/
RECOMP_HOOK_RETURN("ConsoleLogo_Init") void on_ConsoleLogo_Init()
{
    music_rando_begin();
}

/*
    mmrs_loader_init()
        Initialize, update, and load the MMRS database.
*/
RECOMP_CALLBACK("magemods_audio_api", AudioApi_Init) bool mmrs_loader_init()
{
    logLevel = set_log_level(recomp_get_config_u32("log_level"));
    zsoundTable = vec_init(MAX_ZSOUND_SIZE);

    log_info("Loading MMRS files...\n");
    const char *dbPath = "assets/musicDB.db";

    bool success = sql_init(dbPath);

    if(!success)
    {
        return false;
    }

    numMmrs = read_mmrs_files();

    if (numMmrs == -1)
    {
        log_error("Error: Could not initialize MMRS database.");
        return -1;
    }

    allMmrs = recomp_alloc(sizeof(MMRS) * numMmrs);
    log_info("Number of MMRS sequences: %i\n", numMmrs);
    Lib_MemSet(allMmrs, 0, sizeof(MMRS) * numMmrs);

    load_mmrs_table(allMmrs);

    Zseq *zseq;
    Zbank *zbank;

    AudioTableEntry *mySeq;
    AudioTableEntry *bankEntry;


    for (int i = 0; i < numMmrs; i++)
    {
        zseq = recomp_alloc(sizeof(Zseq));
        bool loaded = load_zseq(zseq, allMmrs[i].zseqId);
        bool is_fanfare = false;

        if (!loaded)
        {
            log_error("Error: Failed to load zseq %s.\n", allMmrs[i].songName);
            continue;
        }
        else
        {
            // log_debug("Loaded zseq!\n");
        }

        // log_debug("\nSize is %d", allMmrs[i].zseq.size);
        if (zseq->size == 0) 
        {
            continue;
        }

        mySeq = recomp_alloc(sizeof(AudioTableEntry));

        mySeq->romAddr = (uintptr_t) &zseq->data[0];
        mySeq->size = zseq->size;
        mySeq->medium = MEDIUM_CART;
        mySeq->cachePolicy = CACHE_EITHER;
        mySeq->shortData1 = 0;
        mySeq->shortData2 = 0;
        mySeq->shortData3 = 0;

        s32 sequenceId = AudioApi_AddSequence(mySeq);

        if (allMmrs[i].bankInfoId != -1)
        {
            zbank = recomp_alloc(sizeof(Zbank));
            bankEntry = recomp_alloc(sizeof(AudioTableEntry));

            if(load_zbank(zbank, allMmrs[i].bankInfoId))
            {
                s32 bankNo = AudioApi_ImportVanillaSoundFont(
                    (uintptr_t*)&(zbank->bankData[0]),          // Addr
                    zbank->metaData[2],                         // sampleBank0
                    zbank->metaData[3],                         // sampleBank1
                    zbank->metaData[4],                         // numInstruments
                    zbank->metaData[5],                         // numDrums
                    zbank->metaData[6]                          // numSfx
                );

                allMmrs[i].bankNo = bankNo;
            }
            else
            {
                log_error("Could not load zbank.");
            }

            recomp_free(bankEntry);
        }

        int numZsound = count_zsound(allMmrs[i].id);

        if (numZsound > 0)
        {
            Zsound* this_zsoundTable = recomp_alloc(sizeof(Zsound) * numZsound);
            log_debug("=====================================================\nLoading sound for song %s.\n", allMmrs[i].songName)
            log_debug("Allocated size for %d zsounds, starting from address %p.\n", numZsound, this_zsoundTable);

            if (!load_zsounds(this_zsoundTable, allMmrs[i].id))
            {
                log_error("Error loading custom samples for MMRS %s. Song will be skipped.\n", allMmrs[i].songName);
                continue;
            }
            else
            {
                log_debug("Successfully loaded sounds for MMRS %s!\n", allMmrs[i].songName);
            }
            
            for (int s = 0; s < numZsound; s++)
            {
                vec_push_back(zsoundTable, this_zsoundTable[s].data);
                log_debug("Pushed back to address %p...\n", zsoundTable->dataStart + (MAX_ZSOUND_SIZE * (zsoundTable->numElements - 1)));
                // char* the = recomp_alloc(MAX_ZSOUND_SIZE);
                // vec_at(zsoundTable, zsoundTable->numElements - 1, the);
                // print_bytes(the, 64);
                // recomp_free(the);

                zsound_key_add(this_zsoundTable[s].sampleAddr, (uintptr_t)zsoundTable->dataStart + (MAX_ZSOUND_SIZE * (zsoundTable->numElements - 1)));
                log_debug("Added key %x...... \n", this_zsoundTable[s].sampleAddr);
                print_bytes((void*)zsound_key_lookup(this_zsoundTable[s].sampleAddr), 64);
            }

            log_debug("Bank number: %d\n", allMmrs[i].bankNo);
            log_debug("Custom sounds: %d\n", numZsound);

            CustomSoundFont* font = (CustomSoundFont*)(gAudioCtx.soundFontTable->entries[allMmrs[i].bankNo].romAddr);

            if (logLevel >= LOG_DEBUG)
            {
                print_bytes(&(gAudioCtx.soundFontTable->entries[allMmrs[i].bankNo]), sizeof(AudioTableEntry));
            }

            log_debug("\n");

            Drum* drum;
            SoundEffect* soundEffect;
            Instrument* inst;
            s32 d;

            if (logLevel >= LOG_DEBUG)
            {
                print_bytes(font, sizeof(CustomSoundFont));
            }

            log_debug("\nType   : %x\n", font->type);
            log_debug("Bank1  : %x\n", font->sampleBank1);
            log_debug("Bank2  : %x\n", font->sampleBank2);
            log_debug("nInsts : %x\n", font->numInstruments);
            log_debug("nDrums : %x\n", font->numDrums);
            log_debug("maxInst: %x\n", font->instrumentsCapacity);
            log_debug("maxDrum: %x\n", font->drumsCapacity);
            log_debug("maxSFX : %x\n", font->sfxCapacity);
            log_debug("Insts  : %x\n", font->instruments);
            log_debug("Drums  : %x\n", font->drums);
            log_debug("SFX    : %x\n", font->soundEffects);

            // Print addresses of drums, SFX, instruments
            for (d = 0; d < font->numDrums; d++) {
                drum = font->drums[d];
                if (drum == NULL) {
                    log_debug("Drum %d is null, skipping.\n", d)
                    continue;
                }

                log_debug("Drum: %d\n", d);
                log_debug(" - sample: %p is ", drum->tunedSample.sample->sampleAddr);

                u32 sampleAddr = zsound_key_lookup((uintptr_t)drum->tunedSample.sample->sampleAddr);

                if(sampleAddr)
                {
                    drum->tunedSample.sample->sampleAddr = (u8*)sampleAddr;
                    log_debug("custom.\n");
                }
                else
                {
                    log_debug("vanilla.\n");
                }
            }

            for (d = 0; d < font->numSfx; d++) {
                soundEffect = &font->soundEffects[d];
                if (soundEffect == NULL || soundEffect->tunedSample.sample == NULL) {
                    continue;
                }

                log_debug("Sfx: %d\n", d);
                log_debug(" - sample: %p\n", soundEffect->tunedSample.sample->sampleAddr);

                u32 sampleAddr = zsound_key_lookup((uintptr_t)soundEffect->tunedSample.sample->sampleAddr);

                if(sampleAddr)
                {
                    soundEffect->tunedSample.sample->sampleAddr = (u8*)sampleAddr;
                    log_debug("custom.\n");
                }
                else
                {
                    log_debug("vanilla.\n");
                }
            }

            for (d = 0; d < font->numInstruments; d++) 
            {
                inst = font->instruments[d];
                if (inst == NULL) {
                    continue;
                }

                log_debug("Instrument: %d\n", d);
                log_debug(" - sample: %p is ", inst->normalPitchTunedSample.sample->sampleAddr);

                u32 sampleAddr = zsound_key_lookup((uintptr_t)inst->normalPitchTunedSample.sample->sampleAddr);

                if(sampleAddr)
                {
                    inst->normalPitchTunedSample.sample->sampleAddr = (u8*)sampleAddr;
                    log_debug("custom.\n");
                    if (logLevel >= LOG_DEBUG)
                    {
                        print_bytes(inst->normalPitchTunedSample.sample->sampleAddr, 64);
                        log_debug("\n");
                    }
                }
                else
                {
                    log_debug("vanilla.\n");
                }

                // Add this later
                if (inst->normalRangeLo != 0) {
                    log_debug(" - Low sample: %p is\n", inst->lowPitchTunedSample.sample->sampleAddr);
                    
                    u32 sampleAddr = zsound_key_lookup((uintptr_t)inst->lowPitchTunedSample.sample->sampleAddr);

                    if(sampleAddr)
                    {
                        inst->lowPitchTunedSample.sample->sampleAddr = (u8*)sampleAddr;
                        log_debug("custom.\n");
                        if (logLevel >= LOG_DEBUG)
                        {
                            print_bytes(inst->lowPitchTunedSample.sample->sampleAddr, 64);
                            log_debug("\n");
                        }
                    }
                    else
                    {
                        log_debug("vanilla.\n");
                    }
                }
                if (inst->normalRangeHi != 0x7F) {
                    log_debug(" - High sample: %p is \n", inst->highPitchTunedSample.sample->sampleAddr);

                    u32 sampleAddr = zsound_key_lookup((uintptr_t)inst->highPitchTunedSample.sample->sampleAddr);

                    if(sampleAddr)
                    {
                        inst->highPitchTunedSample.sample->sampleAddr = (u8*)sampleAddr;
                        log_debug("custom.\n");
                        if (logLevel >= LOG_DEBUG)
                        {
                            print_bytes(inst->highPitchTunedSample.sample->sampleAddr, 64);
                            log_debug("\n");
                        }
                    }
                }
            }

            for (int s = 0; s < numZsound; s++)
            {
                log_debug("%x: %x\n", this_zsoundTable[s].sampleAddr, zsound_key_lookup(this_zsoundTable[s].sampleAddr));
                print_bytes((u8*)zsound_key_lookup(this_zsoundTable[s].sampleAddr), 64);
                log_debug("\n");
            }

            for (int s = 0; s < numZsound; s++)
            {
                zsound_key_remove(this_zsoundTable[s].sampleAddr);
            }
        }
        // AudioApi_ReplaceSequence(sequenceId, mySeq);
        AudioApi_AddSequenceFont(sequenceId, allMmrs[i].bankNo);
        log_info("Successfully added sequence %s", allMmrs[i].songName);
        log_debug(", uses bank %i", allMmrs[i].bankNo);

        if (*(u32*)&(allMmrs[i].categories[8]))
        {
            AudioApi_SetSequenceFlags(sequenceId, SEQ_FLAG_FANFARE);
            log_debug(" and is a fanfare\n");
        }
        else
        {
            log_debug(" and is not a fanfare");
        }

        log_info("!\n\n");

        recomp_free(mySeq);
    }

    sql_teardown();

    mmrs_reader_done(allMmrs, numMmrs);

    return true;
}

RECOMP_CALLBACK("magemods_audio_api", AudioApi_SoundFontLoaded) bool mmrs_loader_font_loaded(s32 fontId, u8* fontData)
{
    log_debug("loaded font: %d %p\n", fontId, fontData);
    return true;
}

s32 currSeqId;
const char currSongName[256];
OSTime lastSongLoadTime;

RECOMP_HOOK("AudioLoad_SyncLoadSeq") void prepare_display_song_name(s32 seqId)
{
    currSeqId = seqId;
}

RECOMP_HOOK("Play_PostWorldDraw") void drawSongName(PlayState* this)
{
    if (osGetTime() > lastSongLoadTime + OS_USEC_TO_CYCLES(5 * 1000 * 1000))
    {
        return;
    }

    GfxPrint songNamePrinter;
    Gfx* gfx;

    OPEN_DISPS(this->state.gfxCtx);

    gfx = POLY_OPA_DISP + 1;
    gSPDisplayList(OVERLAY_DISP++, gfx);
    GfxPrint_Init(&songNamePrinter);
    GfxPrint_Open(&songNamePrinter, gfx);

    GfxPrint_SetColor(&songNamePrinter, 200, 200, 200, 255);

    GfxPrint_SetPos(&songNamePrinter, 0, 29);

    vec_at(songNames, randomizedIds[currSeqId], currSongName);

    GfxPrint_Printf(&songNamePrinter, currSongName);
    if (!currSongName[0] != '\0' && logLevel >= LOG_DEBUG)
    {
        GfxPrint_Printf(&songNamePrinter, "(slot 0x%02x --> song 0x%02x)", currSeqId, randomizedIds[currSeqId] + 128);
    }

    gfx = GfxPrint_Close(&songNamePrinter);
    GfxPrint_Destroy(&songNamePrinter);

    gSPEndDisplayList(gfx++);
    gSPBranchList(POLY_OPA_DISP, gfx);
    POLY_OPA_DISP = gfx;

    CLOSE_DISPS(this->state.gfxCtx);
}

RECOMP_HOOK_RETURN("AudioLoad_SyncLoadSeq") void displaySongName()
{
    vec_at(songNames, currSeqId, currSongName);
    lastSongLoadTime = osGetTime();
}

extern u8 sPrevSeqMode;

#include "overlays/actors/ovl_En_Test3/z_en_test3.h"
extern void Player_Action_21(Player* this, PlayState* play);
extern void Player_Action_84(Player* this, PlayState* play);
extern void Player_Action_52(Player* this, PlayState* play);
extern void Player_Action_53(Player* this, PlayState* play);
extern bool func_8082EF20(Player* this);

RECOMP_PATCH void Player_UpdateCamAndSeqModes(PlayState* play, Player* this) {
    u8 seqMode;
    s32 pad[2];
    Camera* camera;
    s32 camMode;

    if (this == GET_PLAYER(play)) {
        seqMode = SEQ_MODE_DEFAULT;
        if (this->stateFlags1 & PLAYER_STATE1_100000) {
            seqMode = SEQ_MODE_STILL;
        } else if (this->csAction != PLAYER_CSACTION_NONE) {
            Camera_ChangeMode(Play_GetCamera(play, CAM_ID_MAIN), CAM_MODE_NORMAL);
        } else {
            camera = (this->actor.id == ACTOR_PLAYER) ? Play_GetCamera(play, CAM_ID_MAIN)
                                                      : Play_GetCamera(play, ((EnTest3*)this)->subCamId);
            if ((this->actor.parent != NULL) && (this->stateFlags3 & PLAYER_STATE3_FLYING_WITH_HOOKSHOT)) {
                camMode = CAM_MODE_HOOKSHOT;
                Camera_SetViewParam(camera, CAM_VIEW_TARGET, this->actor.parent);
            } else if (Player_Action_21 == this->actionFunc) {
                camMode = CAM_MODE_STILL;
            } else if (this->stateFlags3 & PLAYER_STATE3_8000) {
                if (this->stateFlags1 & PLAYER_STATE1_8000000) {
                    camMode = CAM_MODE_GORONDASH;
                } else {
                    camMode = CAM_MODE_FREEFALL;
                }
            } else if (this->stateFlags3 & PLAYER_STATE3_80000) {
                if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
                    camMode = CAM_MODE_GORONDASH;
                } else {
                    camMode = CAM_MODE_GORONJUMP;
                }
            } else if (this->stateFlags2 & PLAYER_STATE2_100) {
                camMode = CAM_MODE_PUSHPULL;
            } else if (this->focusActor != NULL) {
                if (CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_TALK)) {
                    camMode = CAM_MODE_TALK;
                } else if (this->stateFlags1 & PLAYER_STATE1_FRIENDLY_ACTOR_FOCUS) {
                    if (this->stateFlags1 & PLAYER_STATE1_ZORA_BOOMERANG_THROWN) {
                        camMode = CAM_MODE_FOLLOWBOOMERANG;
                    } else {
                        camMode = CAM_MODE_FOLLOWTARGET;
                    }
                } else {
                    camMode = CAM_MODE_BATTLE;
                }
                Camera_SetViewParam(camera, CAM_VIEW_TARGET, this->focusActor);
            } else if (this->stateFlags1 & PLAYER_STATE1_CHARGING_SPIN_ATTACK) {
                camMode = CAM_MODE_CHARGE;
            } else if (this->stateFlags3 & PLAYER_STATE3_100) {
                camMode = CAM_MODE_DEKUHIDE;
            } else if (this->stateFlags1 & PLAYER_STATE1_ZORA_BOOMERANG_THROWN) {
                camMode = CAM_MODE_FOLLOWBOOMERANG;
                Camera_SetViewParam(camera, CAM_VIEW_TARGET, this->zoraBoomerangActor);
            } else if (this->stateFlags1 & (PLAYER_STATE1_4 | PLAYER_STATE1_2000 | PLAYER_STATE1_4000)) {
                if (Player_FriendlyLockOnOrParallel(this)) {
                    camMode = CAM_MODE_HANGZ;
                } else {
                    camMode = CAM_MODE_HANG;
                }
            } else if ((this->stateFlags3 & PLAYER_STATE3_2000) && (this->actor.velocity.y < 0.0f)) {
                if (this->stateFlags1 & (PLAYER_STATE1_PARALLEL | PLAYER_STATE1_LOCK_ON_FORCED_TO_RELEASE)) {
                    camMode = CAM_MODE_DEKUFLYZ;
                } else {
                    camMode = CAM_MODE_DEKUFLY;
                }
            } else if (this->stateFlags1 & (PLAYER_STATE1_PARALLEL | PLAYER_STATE1_LOCK_ON_FORCED_TO_RELEASE)) {
                if (func_800B7128(this) || func_8082EF20(this)) {
                    camMode = CAM_MODE_BOWARROWZ;
                } else if (this->stateFlags1 & PLAYER_STATE1_200000) {
                    camMode = CAM_MODE_CLIMBZ;
                } else {
                    camMode = CAM_MODE_TARGET;
                }
            } else if ((this->stateFlags1 & PLAYER_STATE1_400000) && (this->transformation != 0)) {
                camMode = CAM_MODE_STILL;
            } else if (this->stateFlags1 & PLAYER_STATE1_40000) {
                camMode = CAM_MODE_JUMP;
            } else if (this->stateFlags1 & PLAYER_STATE1_200000) {
                camMode = CAM_MODE_CLIMB;
            } else if (this->stateFlags1 & PLAYER_STATE1_80000) {
                camMode = CAM_MODE_FREEFALL;
            } else if (((Player_Action_84 == this->actionFunc) &&
                        (this->meleeWeaponAnimation >= PLAYER_MWA_FORWARD_SLASH_1H) &&
                        (this->meleeWeaponAnimation <= PLAYER_MWA_ZORA_PUNCH_KICK)) ||
                       (this->stateFlags3 & PLAYER_STATE3_8) ||
                       ((Player_Action_52 == this->actionFunc) && (this->av2.actionVar2 == 0)) ||
                       (Player_Action_53 == this->actionFunc)) {
                camMode = CAM_MODE_STILL;
            } else {
                camMode = CAM_MODE_NORMAL;
                if ((this->speedXZ == 0.0f) &&
                    (!(this->stateFlags1 & PLAYER_STATE1_800000) || (this->rideActor->speed == 0.0f))) {
                    seqMode = SEQ_MODE_STILL;
                }
            }

            Camera_ChangeMode(camera, camMode);
        }

        if (!recomp_get_config_u32("disable_enemy_bgm"))
        {
            if (play->actorCtx.attention.bgmEnemy != NULL) {
                seqMode = SEQ_MODE_ENEMY;
                Audio_UpdateEnemyBgmVolume(sqrtf(play->actorCtx.attention.bgmEnemy->xyzDistToPlayerSq));
            }
        }

        Audio_SetSequenceMode(seqMode);
    }
}