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

RECOMP_IMPORT(".", bool sql_init(const char *dbPath));
RECOMP_IMPORT(".", int read_mmrs_files());
RECOMP_IMPORT(".", bool load_mmrs_table(MMRS* allMmrs));
RECOMP_IMPORT(".", bool load_zseq(Zseq* zseqAddr, int zseqId));
RECOMP_IMPORT(".", bool load_zbank(Zbank* zbankAddr, int zbankId));
RECOMP_IMPORT(".", bool sql_teardown());

MMRS *allMmrs;
int numMmrs;


/*
    mmrs_loader_init()
        Initialize, update, and load the MMRS database.
*/
RECOMP_CALLBACK("magemods_audio_api", AudioApi_Init) bool mmrs_loader_init() 
{
    recomp_printf("Starting mmrs_loader_init()...\n");
    const char *dbPath = "assets/musicDB.db";

    bool success = sql_init(dbPath);

    if(!success)
    {
        return false;
    }

    numMmrs = read_mmrs_files();

    if (numMmrs == -1)
    {
        recomp_printf("\n");
        return -1;
    }

    allMmrs = recomp_alloc(sizeof(MMRS) * numMmrs);
    recomp_printf("Number of MMRS sequences: %i\n", numMmrs);
    Lib_MemSet(allMmrs, 0, sizeof(MMRS) * numMmrs);

    load_mmrs_table(allMmrs);

    // Testing - Remove later
    Zseq *zseq;
    Zbank *zbank;

    recomp_printf("%s\n", allMmrs[0].songName);

    AudioTableEntry *mySeq;
    AudioTableEntry *bankEntry;


    for (int i = 0; i < numMmrs; i++)
    {
        zseq = recomp_alloc(sizeof(Zseq));
        bool loaded = load_zseq(zseq, allMmrs[i].zseqId);
        recomp_printf("%s\n", allMmrs[i].songName);

        if (!loaded)
        {
            recomp_printf("Failed to load zseq.\n");
        }
        else
        {
            recomp_printf("Loaded zseq!\n");
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

        recomp_printf("\nCreated audiotable thing for song %d: %s!\n", i + 1, allMmrs[i].songName);
        recomp_printf("Size of data: %i\n", mySeq->size);
        recomp_printf("===Data spans from %p to %p==\n", mySeq->romAddr, mySeq->romAddr + mySeq->size);
        recomp_printf("AudioTableEntry data: ");
        for (int q = 0; q < 16; q++)
        {
            recomp_printf("%02x ", *((unsigned char*)mySeq->romAddr + q));
        }
        recomp_printf("...\n");

        recomp_printf("bankInfoId: %i\n", allMmrs[i].bankInfoId);

        AudioApi_AddSequence(mySeq);

        if (!strcmp(allMmrs[i].songName, "DELTARUNE - Raise Up Your Bat!"))
        {
            AudioApi_ReplaceSequence(NA_BGM_FILE_SELECT, mySeq);
            recomp_printf("%s", allMmrs[i].songName);
            recomp_printf("Replaced sequence successfully!\n");

            zbank = recomp_alloc(sizeof(Zbank));
            bankEntry = recomp_alloc(sizeof(AudioTableEntry));

            if(load_zbank(zbank, allMmrs[i].bankInfoId))
            {
                bankEntry->romAddr = (uintptr_t) &zbank->bankData[0];
                bankEntry->size = zbank->bankSize;
                bankEntry->medium = zbank->metaData[0];
                bankEntry->cachePolicy = 42;
                bankEntry->shortData1 = (zbank->metaData[2] << 8) | (zbank->metaData[3]);
                bankEntry->shortData2 = (zbank->metaData[4] << 8) | (zbank->metaData[5]);
                bankEntry->shortData3 = (u16) zbank->metaData[6];

                for (int d = 0; d < 32; d++)
                {
                    recomp_printf("%02x ", zbank->bankData[d]);
                }
                recomp_printf("\n");
                
                for (int d = 0; d < 8; d++)
                {
                    recomp_printf("%02x ", zbank->metaData[d]);
                }
                recomp_printf("\n");

                s32 bankNo = AudioApi_AddSoundFont(bankEntry);
                
                AudioApi_ReplaceSequenceFont(NA_BGM_FILE_SELECT, 0, bankNo);
                recomp_printf("Replaced sequence font successfully! (bank 0x%x)\n", bankNo);

                recomp_printf("\n");
            }
            else
            {
                recomp_printf("Could not load zbank.");
            }

            recomp_free(bankEntry);
            recomp_free(mySeq);
        }
    }

    sql_teardown();
}

RECOMP_HOOK("AudioHeap_Init") void AudioHeap_Debug()
{
    // recomp_printf("\nCalled AudioHeap_Debug()\n");
    // recomp_printf("\n==SAMPLING FREQUENCY: %i==\n", gAudioSpecs[gAudioCtx.specId].samplingFreq);
}
