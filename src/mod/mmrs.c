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
RECOMP_IMPORT("debugprinter", void Debug_Print_Draw());

MMRS *allMmrs;
int numMmrs;

int logLevel;

/*
    mmrs_loader_init()
        Initialize, update, and load the MMRS database.
*/
RECOMP_CALLBACK("magemods_audio_api", AudioApi_Init) bool mmrs_loader_init()
{
    logLevel = set_log_level(LOG_INFO);

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
                bankEntry->cachePolicy = 42;
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