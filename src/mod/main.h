#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "logging.h"
#include "modtrackdefs.h"
#include "audio_api/all.h"

#define NUM_SONG_SLOTS 0x80

RECOMP_IMPORT("*", unsigned char* recomp_get_mod_folder_path());

RECOMP_IMPORT(".", int prepare_database(unsigned char* modPath));
RECOMP_IMPORT(".", int prepare_seed(int randoSeed, unsigned char* savePath, bool use_custom, bool use_vanilla));

RECOMP_IMPORT(".", void fetch_randomized_track(int slotIdx, cTrack* modTrack));
RECOMP_IMPORT(".", void fetch_seq(int id, char* dst, size_t size));
RECOMP_IMPORT(".", void fetch_bank(int id, char* dst, size_t size));
RECOMP_IMPORT(".", void fetch_sound(int id, char* dst, size_t size));

RECOMP_IMPORT(".", u32 get_current_time());

RECOMP_IMPORT("mm_recomp_rando", u32 rando_get_random_seed_external());

RECOMP_DECLARE_EVENT(music_rando_begin_randomization());
RECOMP_DECLARE_EVENT(music_rando_launch_menu());
RECOMP_DECLARE_EVENT(music_rando_seed_prepared(cTrack* randomizedAddr));
RECOMP_DECLARE_EVENT(music_rando_randomization_complete(cTrack* randomizedAddr));

typedef struct MusicRando_Config_t {
    unsigned long randomization_off;
    unsigned long track_types;
    // unsigned long disable_randomize_suns_song;
} MusicRando_Config;

MusicRando_Config config;

GameState* playCtx;

Logger logger;
cTrack randomized[NUM_SONG_SLOTS];
cSound customSounds[256];
AudioTable* vanillaTableCopy;
bool randomization_complete;
bool is_existing_seed;

u8 maxVolChannelIdx[SEQ_PLAYER_MAX];
f32 channelVolumeRatios[SEQ_PLAYER_MAX][16];
f32 maxChanVol;

extern bool should_skip_song_title_display[0x7F];

AudioTable* OoTSoundTable;
AudioTable* OoTBankTable;

bool found_oot_audiobin;

AudioTableEntry* create_seq_entry_from_track(cTrack* track);
AudioTable* copy_vanilla_audiotable();

extern void create_startup_menu();
void music_rando_update_db();

extern bool prepare_oot_audiotables();

void prepare_tracks();
void populate_custom_track(cTrack* track);
void populate_vanilla_track(cTrack* track);

void replace_tracks();
void replace_custom(int i);
void replace_vanilla(int i);

extern void update_music_state(PlayState* play);
extern void apply_mask();

extern void log_opts_setup();