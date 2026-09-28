#include "formmask.h"

void update_music_state(PlayState* play)
{
    u16 state = 0;
    Player* player = GET_PLAYER(play);

    if (player) 
    {
        musicState.playerForm = 1 << player->transformation;
        musicState.is_indoors = check_indoors(play->sceneId);
        musicState.is_cave = check_cave(play->sceneId);
        musicState.is_epona = player->stateFlags1 & PLAYER_STATE1_EPONA;
        musicState.is_swim = player->stateFlags1 & PLAYER_STATE1_SWIM;
        musicState.is_spike_rolling = player->stateFlags3 & PLAYER_STATE3_SPIKE_ROLL;
        musicState.is_combat = play->actorCtx.attention.bgmEnemy ? true : false;
        musicState.is_critical_health = LifeMeter_IsCritical();
        musicState.is_day = gSaveContext.save.isNight ? false : true;
    }
    state |= musicState.playerForm;
    state |= musicState.is_indoors ? INDOORS : OUTDOORS;
    state |= musicState.is_cave ? CAVE : 0;
    state |= musicState.is_epona ? EPONA : 0;
    state |= musicState.is_swim ? SWIM : 0;
    state |= musicState.is_spike_rolling ? SPIKES : 0;
    state |= musicState.is_combat ? COMBAT : 0;
    state |= musicState.is_critical_health ? CRITICAL_HEALTH : 0;
    state |= musicState.is_day ? DAY : NIGHT;
    state &= 0x7FFF;

    musicState.playState = state;
    if (musicState.prevPlayState != musicState.playState)
    {
        sprintf_binary(musicState.state_str, state);
        logger.debug("Music state: %s\n", musicState.state_str);
    }
    musicState.prevPlayState = state;
}

void apply_mask()
{
    for (int playerIdx = 0; playerIdx < SEQ_PLAYER_MAX; playerIdx++)
    {
        if (gActiveSeqs[playerIdx].seqId < 2 || gActiveSeqs[playerIdx].seqId > 0x7F) { continue; }

        cFormMask* currMask = &musicState.nowPlaying[playerIdx]->formmask;
        
        for (int channelIdx = 0; channelIdx < 16; channelIdx++)
        {
            u16 channelMask = currMask->states[channelIdx] & 0x7FFF;
            u16 channelState = 0;

            for (int state = 0; state < 15; state++)
            {
                if (channelMask & musicState.playState)
                {
                    channelState = currMask->cumulativeStates & 1 << state ?
                        channelState | 1 << state : 1 << state;
                }
            }
            musicState.prevChannelStates[channelIdx] = musicState.channelStates[channelIdx];
            musicState.channelStates[channelIdx] = channelState;
            channelState &= 0x7FFF;
        }
        for (int i = 0; i < 16; i++)
        {
            SequenceChannel* channel = gAudioCtx.seqPlayers[playerIdx].channels[i];
            if (musicState.channelStates[i] & musicState.playState)
            {
                if (channel->muted)
                {
                    channel->muted = false;
                }
            }
            else
            {
                if (!channel->muted)
                {
                    channel->muted = true;
                }
            }
        }
    }


}

bool check_indoors(int sceneId)
{
    switch (sceneId) {
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
            return true;
    }

    return false;
}

bool check_cave(int sceneId)
{
    switch(sceneId)
    {
        case SCENE_KAKUSIANA: // Grottos
        case SCENE_DEKUTES: // Deku Scrub Playground
        case SCENE_YOUSEI_IZUMI: // Fairy's Fountain
        case SCENE_GORON_HAKA: // Goron Graveyard
            return true;
    }

    return false;
}

// AudioSeq_GetActiveSeqId() pulls from gActiveSeqs, which does not get update on AudioLoad_SyncLoadSeq()
// or AudioLoad_SyncInitSeqPlayer(). This means Clock Town is always at 0x1d (the morning sequence).
// so we update the value in gActiveSeqs. Does not seem to have ill effects?
RECOMP_HOOK("AudioLoad_SyncInitSeqPlayer") void update_activeseqs_seq_id(s32 playerIndex, s32 seqId, s32 arg2)
{
    if (!logger.is_initialized) { return; }
    gActiveSeqs[playerIndex].seqId = seqId;
    musicState.nowPlaying[playerIndex] = &(randomized[seqId]);
    if (seqId < 0x80)
    {
        musicState.isPlayerActive[playerIndex] = true;
        musicState.nowPlaying[playerIndex] = &randomized[seqId];
    }
    else
    {
        musicState.isPlayerActive[playerIndex] = false;
        musicState.nowPlaying[playerIndex] = NULL;
    }
    return;
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