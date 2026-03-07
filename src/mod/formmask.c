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
    state |= musicState.is_combat ? COMBAT : 0;
    state |= musicState.is_critical_health ? CRITICAL_HEALTH : 0;
    state |= musicState.is_day ? DAY : NIGHT;

    musicState.state = state;
    if (musicState.prevState != musicState.state)
    {
        logger.debug("Formmask: %x\n", musicState.state);
    }
    musicState.prevState = state;
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