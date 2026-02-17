#include "main.h"
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

RECOMP_DECLARE_EVENT(music_rando_db_updated())

Logger logger;
cTrack randomized[NUM_SONG_SLOTS];

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
        logger.dev("Test\n");
        randoSeed = rando_get_random_seed_external();
        logger.dev("ing\n");
    }
    else
    {
        randoSeed = get_current_time();
    }

    prepare_seed(randoSeed, savePath, true, true);

    prepare_tracks();

    logger.info("");
    for (int i = 2; i < NUM_SONG_SLOTS; i++)
    {
        logger.noheader.info("Randomized %s to %s!\n", randomized[i].slotName, randomized[i].name);
    }
}

void prepare_tracks()
{
    // Start at 2, because we want to skip the SFX and Ambience entries.
    for (int i = 2; i < NUM_SONG_SLOTS; i++)
    {
        fetch_randomized_track(i, &randomized[i]);
        if (randomized[i].hasSeq)
        {
            randomized[i].seq.data = recomp_alloc(randomized[i].seq.size);
            fetch_seq(randomized[i].seq.id, randomized[i].seq.data, randomized[i].seq.size);
        }
        if (randomized[i].hasBank)
        {
            randomized[i].bank.data = recomp_alloc(randomized[i].bank.size);
            fetch_bank(randomized[i].bank.id, randomized[i].bank.data, randomized[i].bank.size);
        }
        for (int s = 0; s < randomized[i].numSounds; i++)
        {
            randomized[i].sounds[s].data = recomp_alloc(randomized[i].sounds[s].size);
            fetch_sound(randomized[i].sounds[s].id, randomized[i].sounds[s].data, randomized[i].sounds[s].size);
        }
    }
}