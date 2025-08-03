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
#include "audio_api.h"

// Project Files
#include "read_mmrs.h"

// RECOMP_IMPORT("magemods_audio_api", void AudioApi_ReplaceSequence(AudioTableEntry entry, s32 seqId));
// RECOMP_IMPORT("magemods_audio_api", void AudioApi_ReplaceSequenceFont(s32 seqId, s32 fontNum, s32 fontId));

RECOMP_IMPORT(".", int read_mmrs_files(const char *dbPath));
RECOMP_IMPORT(".", bool load_mmrs_table(const char *dbPath, MMRS* allMmrs));
RECOMP_IMPORT(".", bool load_zseq(const char *dbPath, Zseq* zseqAddr, int zseqId));
RECOMP_IMPORT(".", void sql_teardown());

MMRS *allMmrs;
int numMmrs;


/*
    mmrs_loader_init()
        Initialize, update, and load the MMRS database.
*/
RECOMP_CALLBACK("magemods_audio_api", AudioApi_Init) void mmrs_loader_init() 
{
    const char *dbPath = "assets/musicDB.db";

    numMmrs = read_mmrs_files(dbPath);

    allMmrs = recomp_alloc(sizeof(MMRS) * numMmrs);
    recomp_printf("Number of MMRS sequences: %i", numMmrs);
    Lib_MemSet(allMmrs, 0, sizeof(MMRS) * numMmrs);

    load_mmrs_table(dbPath, allMmrs);

    // Testing - Remove later
    Zseq *zseq = recomp_alloc(sizeof(Zseq) * numMmrs);

    recomp_printf("%s\n", allMmrs[0].songName);

    AudioTableEntry mySeq;

    for (int i = 0; i < numMmrs; i++)
    {
        bool loaded = load_zseq(dbPath, zseq, i + 1);
        recomp_printf("%s\n", allMmrs[i].songName);

        if (!loaded)
        {
            recomp_printf("Failed to load zseq.\n");
        }
        else
        {
            recomp_printf("Did the thing\n");
        }

        // recomp_printf("\nSize is %d", allMmrs[i].zseq.size);
        if (zseq->size == 0) 
        {
            continue;
        }

        AudioTableEntry mySeq = 
        {
            (uintptr_t) &zseq->data[0],
            zseq->size,
            MEDIUM_CART,
            CACHE_EITHER,
            0, 0, 0,
        };

        recomp_printf("\nCreated audiotable thing for song %d: %s!\n", i + 1, allMmrs[i].songName);
        recomp_printf("Size of data: %i\n", mySeq.size);
        recomp_printf("===Data spans from %p to %p==\n", mySeq.romAddr, mySeq.romAddr + mySeq.size);
        recomp_printf("AudioTableEntry data: ");
        for (int q = 0; q < 16; q++)
        {
            recomp_printf("%02x ", *((unsigned char*)mySeq.romAddr + q));
        }
        recomp_printf("...\n");

        AudioApi_AddSequence(&mySeq);

        if (!strcmp(allMmrs[i].songName, "ULTRAKILL - The Fire Is Gone"))
        {
            AudioApi_ReplaceSequence(NA_BGM_FILE_SELECT, &mySeq);
            recomp_printf("Replaced sequence successfully!\n");
            AudioApi_ReplaceSequenceFont(NA_BGM_FILE_SELECT, 0, allMmrs[i].bankNo);
            recomp_printf("Replaced sequence font successfully! (bank 0x%x)\n", allMmrs[i].bankNo);
        }
    }

    sql_teardown();
}

RECOMP_HOOK("AudioHeap_Init") void AudioHeap_Debug()
{
    // recomp_printf("\nCalled AudioHeap_Debug()\n");
    // recomp_printf("\n==SAMPLING FREQUENCY: %i==\n", gAudioSpecs[gAudioCtx.specId].samplingFreq);
}
