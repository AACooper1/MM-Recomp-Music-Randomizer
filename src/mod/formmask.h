#ifndef FORMMASK_H
#define FORMMASK_H

#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompui.h"

#include "logging.h"
#include "modtrackdefs.h"

#define PLAYER_STATE1_EPONA PLAYER_STATE1_800000
#define PLAYER_STATE1_SWIM PLAYER_STATE1_8000000
#define PLAYER_STATE3_SPIKE_ROLL PLAYER_STATE3_80000

extern Logger logger;
extern cTrack randomized[0x80];

extern u32 LifeMeter_IsCritical(void);

enum FormMaskState {
    FIERCE_DEITY = 1 << 0x0,
    GORON = 1 << 0x1,
    ZORA = 1 << 0x2,
    DEKU = 1 << 0x3,

    HUMAN = 1 << 0x4,
    OUTDOORS = 1 << 0x5,
    INDOORS = 1 << 0x6,
    CAVE = 1 << 0x7,

    EPONA = 1 << 0x8,
    SWIM = 1 << 0x9,
    SPIKES = 1 << 0xA,
    COMBAT = 1 << 0xB,
    
    CRITICAL_HEALTH = 1 << 0xC,
    DAY = 1 << 0xD,
    NIGHT = 1 << 0xE,
    FORMMASK_UNUSED = 1 << 0xF
};

typedef struct MusicState_t 
{
    bool isPlayerActive[SEQ_PLAYER_MAX];
    cTrack* nowPlaying[SEQ_PLAYER_MAX];
    bool has_formmask;

    u32 playerForm;
    bool is_indoors;
    bool is_cave;
    bool is_epona;
    bool is_swim;
    bool is_spike_rolling;
    bool is_combat;
    bool is_critical_health;
    bool is_day;

    u16 prevPlayState; // For debug printing
    char state_str[20]; // Room for 16 bits + 3 spaces for grouping + \0 at the end
    u16 playState;

    u16 prevChannelStates[16];// For debug printing;
    u16 channelStates[16];
} MusicState;

void update_music_state(PlayState* play);
void apply_mask();

bool check_indoors(int sceneId);
bool check_cave(int sceneId);

MusicState musicState;

#endif