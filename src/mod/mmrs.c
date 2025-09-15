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

#include "audio/lib/seqplayer.c"

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
RECOMP_DECLARE_EVENT(on_apply_formmask(PlayState* play, Player* this))

extern u32 LifeMeter_IsCritical(void);

extern Vector* songNames;
extern int* randomizedIds;
Vector* formmaskAddrs;

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
    formmaskAddrs = vec_init(sizeof(u16*));

    log_info("Number of MMRS sequences: %i\n", numMmrs);

    if (numMmrs == 0)
    {
        return true;
    }

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

        if (zseq->size == 0) 
        {
            continue;
        }

        mySeq = recomp_alloc(sizeof(AudioTableEntry));

        mySeq->romAddr = (uintptr_t) &zseq->data[0];
        mySeq->size = zseq->size;
        mySeq->medium = MEDIUM_CART;
        mySeq->cachePolicy = CACHE_EITHER;
        mySeq->shortData1 = formmaskAddrs->numElements;
        mySeq->shortData2 = 0;
        mySeq->shortData3 = 0;

        s32 sequenceId = AudioApi_AddSequence(mySeq);

        u16* formmaskAddr = &(allMmrs[i].formmask[0]);

        vec_push_back(formmaskAddrs, &formmaskAddr);

        // print_bytes(allMmrs[i].formmask, 34);

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
            // log_debug("=====================================================\nLoading sound for song %s.\n", allMmrs[i].songName)
            // log_debug("Allocated size for %d zsounds, starting from address %p.\n", numZsound, this_zsoundTable);

            if (!load_zsounds(this_zsoundTable, allMmrs[i].id))
            {
                // log_error("Error loading custom samples for MMRS %s. Song will be skipped.\n", allMmrs[i].songName);
                continue;
            }
            else
            {
                // log_debug("Successfully loaded sounds for MMRS %s!\n", allMmrs[i].songName);
            }
            
            for (int s = 0; s < numZsound; s++)
            {
                zsound_key_add(this_zsoundTable[s].sampleAddr, (uintptr_t)this_zsoundTable[s].data);
                // log_debug("Added key %x...... \n", this_zsoundTable[s].sampleAddr);
            }

            // log_debug("Bank number: %d\n", allMmrs[i].bankNo);
            // log_debug("Custom sounds: %d\n", numZsound);

            CustomSoundFont* font = (CustomSoundFont*)(gAudioCtx.soundFontTable->entries[allMmrs[i].bankNo].romAddr);

            // if (logLevel >= LOG_DEBUG)
            // {
            //     print_bytes(&(gAudioCtx.soundFontTable->entries[allMmrs[i].bankNo]), sizeof(AudioTableEntry));
            // }

            // log_debug("\n");

            Drum* drum;
            SoundEffect* soundEffect;
            Instrument* inst;
            s32 d;

            // if (logLevel >= LOG_DEBUG)
            // {
            //     print_bytes(font, sizeof(CustomSoundFont));
            // }

            // log_debug("\nType   : %x\n", font->type);
            // log_debug("Bank1  : %x\n", font->sampleBank1);
            // log_debug("Bank2  : %x\n", font->sampleBank2);
            // log_debug("nInsts : %x\n", font->numInstruments);
            // log_debug("nDrums : %x\n", font->numDrums);
            // log_debug("maxInst: %x\n", font->instrumentsCapacity);
            // log_debug("maxDrum: %x\n", font->drumsCapacity);
            // log_debug("maxSFX : %x\n", font->sfxCapacity);
            // log_debug("Insts  : %x\n", font->instruments);
            // log_debug("Drums  : %x\n", font->drums);
            // log_debug("SFX    : %x\n", font->soundEffects);

            // Print addresses of drums, SFX, instruments
            for (d = 0; d < font->numDrums; d++) {
                drum = font->drums[d];
                if (drum == NULL) {
                    // log_debug("Drum %d is null, skipping.\n", d)
                    continue;
                }

                // log_debug("Drum: %d\n", d);
                // log_debug(" - sample: %p is ", drum->tunedSample.sample->sampleAddr);

                u32 sampleAddr = zsound_key_lookup((uintptr_t)drum->tunedSample.sample->sampleAddr);

                if(sampleAddr)
                {
                    drum->tunedSample.sample->sampleAddr = (u8*)sampleAddr;
                    // log_debug("custom.\n");
                }
                else
                {
                    // log_debug("vanilla.\n");
                }
            }

            for (d = 0; d < font->numSfx; d++) {
                soundEffect = &font->soundEffects[d];
                if (soundEffect == NULL || soundEffect->tunedSample.sample == NULL) {
                    continue;
                }

                // log_debug("Sfx: %d\n", d);
                // log_debug(" - sample: %p\n", soundEffect->tunedSample.sample->sampleAddr);

                u32 sampleAddr = zsound_key_lookup((uintptr_t)soundEffect->tunedSample.sample->sampleAddr);

                if(sampleAddr)
                {
                    soundEffect->tunedSample.sample->sampleAddr = (u8*)sampleAddr;
                    // log_debug("custom.\n");
                }
                else
                {
                    // log_debug("vanilla.\n");
                }
            }

            for (d = 0; d < font->numInstruments; d++) 
            {
                inst = font->instruments[d];
                if (inst == NULL) {
                    continue;
                }

                // log_debug("Instrument: %d\n", d);
                // log_debug(" - sample: %p is ", inst->normalPitchTunedSample.sample->sampleAddr);

                u32 sampleAddr = zsound_key_lookup((uintptr_t)inst->normalPitchTunedSample.sample->sampleAddr);

                if(sampleAddr)
                {
                    inst->normalPitchTunedSample.sample->sampleAddr = (u8*)sampleAddr;
                    // log_debug("custom.\n");
                    // if (logLevel >= LOG_DEBUG)
                    // {
                    //     print_bytes(inst->normalPitchTunedSample.sample->sampleAddr, 64);
                    //     log_debug("\n");
                    // }
                }
                else
                {
                    // log_debug("vanilla.\n");
                }

                // Add this later
                if (inst->normalRangeLo != 0) {
                    // log_debug(" - Low sample: %p is\n", inst->lowPitchTunedSample.sample->sampleAddr);
                    
                    u32 sampleAddr = zsound_key_lookup((uintptr_t)inst->lowPitchTunedSample.sample->sampleAddr);

                    if(sampleAddr)
                    {
                        inst->lowPitchTunedSample.sample->sampleAddr = (u8*)sampleAddr;
                        // log_debug("custom.\n");
                        // if (logLevel >= LOG_DEBUG)
                        // {
                        //     print_bytes(inst->lowPitchTunedSample.sample->sampleAddr, 64);
                        //     log_debug("\n");
                        // }
                    }
                    else
                    {
                        // log_debug("vanilla.\n");
                    }
                }
                if (inst->normalRangeHi != 0x7F) {
                    // log_debug(" - High sample: %p is \n", inst->highPitchTunedSample.sample->sampleAddr);

                    u32 sampleAddr = zsound_key_lookup((uintptr_t)inst->highPitchTunedSample.sample->sampleAddr);

                    if(sampleAddr)
                    {
                        inst->highPitchTunedSample.sample->sampleAddr = (u8*)sampleAddr;
                        // log_debug("custom.\n");
                        // if (logLevel >= LOG_DEBUG)
                        // {
                        //     print_bytes(inst->highPitchTunedSample.sample->sampleAddr, 64);
                        //     log_debug("\n");
                        // }
                    }
                }
            }

            for (int s = 0; s < numZsound; s++)
            {
                // log_debug("%x: %x\n", this_zsoundTable[s].sampleAddr, zsound_key_lookup(this_zsoundTable[s].sampleAddr));
                // print_bytes((u8*)zsound_key_lookup(this_zsoundTable[s].sampleAddr), 64);
                // log_debug("\n");
            }

            for (int s = 0; s < numZsound; s++)
            {
                zsound_key_remove(this_zsoundTable[s].sampleAddr);
            }
        }
        // AudioApi_ReplaceSequence(sequenceId, mySeq);
        AudioApi_AddSequenceFont(sequenceId, allMmrs[i].bankNo);
        // log_debug("Successfully added sequence %s", allMmrs[i].songName);
        // log_debug(", uses bank %i", allMmrs[i].bankNo);

        if (*(u32*)&(allMmrs[i].categories[8]))
        {
            AudioApi_SetSequenceFlags(sequenceId, SEQ_FLAG_FANFARE);
            // log_debug(" and is a fanfare\n");
        }
        else
        {
            // log_debug(" and is not a fanfare");
        }

        // log_debug("!\n");

        recomp_free(mySeq);
    }

    gAudioCtx.sequenceFontTable[NA_BGM_MILK_BAR_DUPLICATE] = gAudioCtx.sequenceFontTable[NA_BGM_MILK_BAR];

    sql_teardown();

    mmrs_reader_done(allMmrs, numMmrs);

    return true;
}

RECOMP_CALLBACK(".", on_apply_formmask) void apply_formmask(PlayState* play, Player* this)
{
    u16 seqId = AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);

    int offsetSeqId = randomizedIds[seqId];
    if (offsetSeqId >= 128)
    {
        offsetSeqId += (((offsetSeqId)/254) * 2);
        offsetSeqId += 128;
    }

    if (offsetSeqId == 29)
    {
        if (randomizedIds[0x15 + gSaveContext.save.day - 1] < 128)
        {
            // log_debug("Okay the thing now is %i. that's the clock town day %i id\n", randomizedIds[0x15 + gSaveContext.save.day - 1], gSaveContext.save.day - 1)
            return;
        }
    }
    else if (offsetSeqId < 128 || seqId % 256 > 253 || offsetSeqId > gAudioCtx.sequenceTable->header.numEntries)
    {
        // log_debug("Not doing the thing now because it um. because it's the randomizedIds[seqId] is %i sorry\n", randomizedIds[seqId])
        return;
    }

    // log_debug("SeqId is %i, randomizedId[seqId] is %i]\n", seqId, offsetSeqId)

    u16** formMaskAddr = recomp_alloc(sizeof(u16*));
    vec_at(formmaskAddrs, gAudioCtx.sequenceTable->entries[offsetSeqId].shortData1, formMaskAddr);

    u16* mask = *formMaskAddr;

    // Get cumulative states
    u16 cumulativeStates  = mask[16];

    // Form
    u16 state = 1 << gSaveContext.save.playerForm;

    // Indoors/Outdoors/Cave
    // Copied from MMR code

    switch (play->sceneId) 
    {
        case SCENE_KAKUSIANA: // Grottos
        case SCENE_DEKUTES: // Deku Scrub Playground
        case SCENE_YOUSEI_IZUMI: // Fairy's Fountain
        case SCENE_GORON_HAKA: // Goron Graveyard
            state = cumulativeStates & CAVE ? state |= CAVE : CAVE;
            break;
        case SCENE_WITCH_SHOP: // Potion Shop
        case SCENE_AYASHIISHOP: // Curiosity Shop
        case SCENE_OMOYA: // Ranch House and Barn
        case SCENE_BOWLING: // Honey and Darling
        case SCENE_SONCHONOIE: // Mayor's Residence
        //case SCENE_MILK_BAR: // Milk Bar
        case SCENE_TAKARAYA: // Treasure Chest Shop
        case SCENE_SYATEKI_MIZU: // Town Shooting Gallery
        case SCENE_SYATEKI_MORI: // Swamp Shooting Gallery
        case SCENE_KAJIYA: // Mountain Smithy
        case SCENE_POSTHOUSE: // Post Office
        case SCENE_LABO: // Marine Research Lab
        case SCENE_8ITEMSHOP: // Trading Post
        case SCENE_TAKARAKUJI: // Lottery Shop
        case SCENE_FISHERMAN: // Fisherman's Hut
        case SCENE_GORONSHOP: // Goron Shop
        case SCENE_BANDROOM: // Zora Hall Rooms
        case SCENE_TOUGITES: // Poe Hut
        case SCENE_DOUJOU: // Swordsman's School
        case SCENE_MAP_SHOP: // Tourist Information
        case SCENE_YADOYA: // Stock Pot Inn
        case SCENE_BOMYA: // Bomb Shop
            state = cumulativeStates & INDOORS ? state |= INDOORS : INDOORS;
            break;
        default:
            state = cumulativeStates & OUTDOORS ? state |= OUTDOORS : OUTDOORS;
        }

    // Epona
    if (this->rideActor)
    {
        state = cumulativeStates & EPONA ? state |= EPONA : EPONA;
    }

    // Swim
    if (this->stateFlags1 & PLAYER_STATE1_8000000 || this->stateFlags3 & PLAYER_STATE3_8000)
    {
        state = cumulativeStates & SWIM ? state |= SWIM : SWIM;
    }

    // Spikes
    if (this->stateFlags3 & PLAYER_STATE3_80000)
    {
        state = cumulativeStates & SPIKES ? state |= SPIKES : SPIKES;
    }

    // Combat
    if (play->actorCtx.attention.bgmEnemy != NULL)
    {
        state = cumulativeStates & COMBAT ? state |= COMBAT : COMBAT;
    }

    // Critical Health
    if (LifeMeter_IsCritical())
    {
        state = cumulativeStates & CRITICAL_HEALTH ? state |= CRITICAL_HEALTH : CRITICAL_HEALTH;
    }

    for (int i = 0; i < 16; i++)
    {
        SequenceChannel* channel = gAudioCtx.seqPlayers[SEQ_PLAYER_BGM_MAIN].channels[i];

        if (state & mask[i])
        {
            channel->muted = false;
        }
        else
        {
            channel->muted = true;
        }
    }

    // log_debug(" %x \n ", state)
    // for (int i = 0; i < 16; i++)
    // {
    //     log_debug("%i ", (mask[2] & 1 << i == 0))
    // }
    // log_debug("\n(")
    // for (int i = 0; i < 16; i++)
    // {
    //     log_debug("%i ", !(gAudioCtx.seqPlayers[SEQ_PLAYER_BGM_MAIN].channels[i]->muted))
    // }
    // log_debug(")\n")

    recomp_free(formMaskAddr);
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
    if (!recomp_get_config_u32("show_song_titles"))
    {
        return;
    }
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

    int offsetSeqId = currSeqId;
    if (currSeqId >= 256)
    {
        offsetSeqId -= (((currSeqId - 256)/254) * 2);
        offsetSeqId -= 128;
    }

    vec_at(songNames, randomizedIds[offsetSeqId], currSongName);

    GfxPrint_Printf(&songNamePrinter, currSongName);
    if (!currSongName[0] != '\0' && logLevel >= LOG_DEBUG)
    {
        GfxPrint_Printf(&songNamePrinter, "(%02x -> %02x)", currSeqId, randomizedIds[currSeqId]);
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

        on_apply_formmask(play, this);

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