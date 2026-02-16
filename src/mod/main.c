#include "main.h"
#include "audio_api/all.h"

#define NUM_SONG_SLOTS 0x80

RECOMP_IMPORT("*", unsigned char* recomp_get_mod_folder_path());
RECOMP_IMPORT(".", int prepare_database(unsigned char* modPath));
RECOMP_IMPORT(".", int prepare_seed(int randoSeed));

RECOMP_IMPORT(".", void fetch_randomized_track(int slotIdx, cTrack* modTrack));
RECOMP_IMPORT(".", void fetch_seq(int id, char* dst, size_t size));
RECOMP_IMPORT(".", void fetch_bank(int id, char* dst, size_t size));
RECOMP_IMPORT(".", void fetch_sound(int id, char* dst, size_t size));

Logger logger;
cTrack randomized[NUM_SONG_SLOTS];

RECOMP_HOOK("ConsoleLogo_Init") void create_db()
{
    logger_init(&logger);
    set_log_level(LOG_DEV);
    logger.debug("And the %c side of it works too!\n\n", 67);

    unsigned char* modPath = recomp_get_mod_folder_path();
    logger.debug("Mod path: %s", modPath);
    logger.dev("sizeof cseq in mod code: %i\n", sizeof(cSequence));

    prepare_database(modPath);
    // Update when rando integration is good
    prepare_seed(0x00);

    prepare_tracks();

    for (int i = 2; i < NUM_SONG_SLOTS; i++)
    {
        logger.noheader.debug("Randomized %s to %s!\n", randomized[i].slotName, randomized[i].name);
    }

    // Testing
    AudioTableEntry* mySeq = recomp_alloc(sizeof(AudioTableEntry));

    cTrack* track = &randomized[2];

    mySeq->romAddr = (uintptr_t) track->seq.data;
    mySeq->size = track->seq.size;
    mySeq->medium = MEDIUM_CART;
    mySeq->cachePolicy = CACHE_EITHER;
    mySeq->shortData1 = 0;
    mySeq->shortData2 = 0;
    mySeq->shortData3 = 0;

    AudioApi_ReplaceSequence(NA_BGM_FILE_SELECT, mySeq);
    if (track->bankNo == 0x28)
    {
        s32 bankNo = AudioApi_ImportVanillaSoundFont(
            (uintptr_t*)track->bank.data,
            track->bank.header[2],
            track->bank.header[3],
            track->bank.header[4],
            track->bank.header[5],
            track->bank.header[6]
        );
        AudioApi_ReplaceSequenceFont(NA_BGM_FILE_SELECT, 0, bankNo);
        logger.dev("Sequence font: %x\nBank Number: %x\n", AudioApi_GetSequenceFont(NA_BGM_FILE_SELECT, 0), bankNo);
    }
    else
    {
        AudioApi_ReplaceSequenceFont(NA_BGM_FILE_SELECT, 0, track->bankNo);
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
            logger.debug("\
                name: %s\n\
                  id: %x\n\
                 hdr: %llx\n\
                data: %p\n\
                ", randomized[i].name, randomized[i].bank.id, (u64)randomized[i].bank.header, randomized[i].bank.data);
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