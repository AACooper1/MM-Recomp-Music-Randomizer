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
RECOMP_IMPORT(".", bool load_mmrs_table(MMRS* allMmrs));
RECOMP_IMPORT(".", bool load_zseq(Zseq* zseqAddr, int zseqId));
RECOMP_IMPORT(".", bool load_zbank(Zbank* zbankAddr, int zbankId));
RECOMP_IMPORT(".", bool sql_teardown());

RECOMP_DECLARE_EVENT(music_rando_begin());
RECOMP_DECLARE_EVENT(mmrs_reader_done(MMRS *allMmrs, int numMmrs));

extern Vector* songNames;
extern int* randomizedIds;

MMRS *allMmrs;
int numMmrs;

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

    log_debug("Starting mmrs_loader_init()...\n");
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

    // Testing - Remove later
    Zseq *zseq;
    Zbank *zbank;

    log_debug("%s\n", allMmrs[0].songName);

    AudioTableEntry *mySeq;
    AudioTableEntry *bankEntry;


    for (int i = 0; i < numMmrs; i++)
    {
        zseq = recomp_alloc(sizeof(Zseq));
        bool loaded = load_zseq(zseq, allMmrs[i].zseqId);
        log_debug("%s\n", allMmrs[i].songName);
        bool is_fanfare = false;

        if (!loaded)
        {
            log_error("Error: Failed to load zseq %s.\n", allMmrs[i].songName);
        }
        else
        {
            log_debug("Loaded zseq!\n");
        }

        // recomp_printf("\nSize is %d", allMmrs[i].zseq.size);
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

        log_debug("\nCreated audiotable thing for song %d: %s!\n", i + 1, allMmrs[i].songName);
        log_debug("Size of data: %i\n", mySeq->size);
        log_debug("===Data spans from %p to %p==\n", mySeq->romAddr, mySeq->romAddr + mySeq->size);
        log_debug("AudioTableEntry data: ");
        for (int q = 0; q < 16; q++)
        {
            log_debug("%02x ", *((unsigned char*)mySeq->romAddr + q));
        }
        log_debug("...\n");

        log_debug("bankInfoId: %i\n", allMmrs[i].bankInfoId);
        log_debug("Categories (below):\n");
        if (logLevel >= LOG_DEBUG)
        {
            print_bytes(allMmrs[i].categories, 256);
        }

        s32 sequenceId = AudioApi_AddSequence(mySeq);
        log_debug("New sequence ID: %i\n", sequenceId);

        if (allMmrs[i].bankInfoId != -1)
        {
            zbank = recomp_alloc(sizeof(Zbank));
            bankEntry = recomp_alloc(sizeof(AudioTableEntry));

            if(load_zbank(zbank, allMmrs[i].bankInfoId))
            {
                bankEntry->romAddr = (uintptr_t) &zbank->bankData[0];
                bankEntry->size = zbank->bankSize;
                bankEntry->medium = zbank->metaData[0];
                bankEntry->cachePolicy = zbank->metaData[1];
                bankEntry->shortData1 = (zbank->metaData[2] << 8) | (zbank->metaData[3]);
                bankEntry->shortData2 = (zbank->metaData[4] << 8) | (zbank->metaData[5]);
                bankEntry->shortData3 = (u16) zbank->metaData[6];

                if (logLevel >= LOG_DEBUG)
                {
                    print_bytes(&zbank->bankData[0], 256);
                }
                log_debug("\n");
                
                for (int d = 0; d < 8; d++)
                {
                    log_debug("%02x ", zbank->metaData[d]);
                }
                log_debug("\n");

                s32 bankNo = AudioApi_AddSoundFont(bankEntry);
                allMmrs[i].bankNo = bankNo;

                log_debug("%d %d %p %p\n", bankNo, gAudioCtx.soundFontTable->entries[bankNo].cachePolicy, 
                    gAudioCtx.soundFontTable->entries[bankNo].romAddr, &zbank->bankData[0]);
                // s32 bankNo = AudioApi_ImportVanillaSoundFont(
                //     (uintptr_t*)&(zbank->bankData[0]),          // Addr
                //     zbank->metaData[2],                         // sampleBank0
                //     zbank->metaData[3],                         // sampleBank1
                //     zbank->metaData[4],                         // numInstruments
                //     zbank->metaData[5],                         // numDrums
                //     zbank->metaData[6]                          // numSfx
                // );

                allMmrs[i].bankNo = bankNo;

            }
            else
            {
                log_error("Could not load zbank.");
            }

            recomp_free(bankEntry);
        }
        AudioApi_ReplaceSequence(sequenceId, mySeq);
        AudioApi_AddSequenceFont(sequenceId, allMmrs[i].bankNo);
        log_debug("Sequence is at ID %i, uses bank %i.\n\n", sequenceId, allMmrs[i].bankNo);

        if (*(u32*)&(allMmrs[i].categories[8]))
        {
            AudioApi_SetSequenceFlags(sequenceId, SEQ_FLAG_FANFARE);
            log_debug("Sequence is a fanfare.\n");
        }

        recomp_free(mySeq);
    }

    sql_teardown();

    mmrs_reader_done(allMmrs, numMmrs);

    return true;
}

RECOMP_CALLBACK("magemods_audio_api", AudioApi_SoundFontLoaded) bool mmrs_loader_font_loaded(s32 fontId, u8* fontData)
{
    log_debug("loaded font: %d %p\n", fontId, fontData);

    if(fontId > 0x28 && logLevel >= LOG_DEBUG)
    {
        print_bytes(fontData, 512);
    }
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