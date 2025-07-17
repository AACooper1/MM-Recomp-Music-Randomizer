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

RECOMP_IMPORT(".", int get_num_mmrs());
RECOMP_IMPORT(".", void read_mmrs_files(MMRS* mmrsAddr));

MMRS *allMmrs;

// Stolen from magemods, who didn't export this function
void print_bytes(void *ptr, int size)
{
    unsigned char *p = ptr;
    int i;
    for (i = 0; i < size; i++) {
        // recomp_printf("%02X ", p[i]);
    }
    // recomp_printf("\n");
}

RECOMP_CALLBACK("magemods_audio_api", AudioApi_Init) void my_mod_on_init() {

    // recomp_printf("grahg\n");

    int numMmrs = get_num_mmrs();
    // recomp_printf("\nTotal no. MMRS: %i\n", numMmrs);

    allMmrs = recomp_alloc(sizeof(MMRS) * numMmrs);
    for (int i = 0; i < numMmrs; i++)
    {
        Zseq *zseq = recomp_alloc(sizeof(Zseq));
        allMmrs[i].zseq = *zseq;
        allMmrs[i].zseq.bankNo = 0x24;
        recomp_printf("sizeof(MMRS) = %i\n", sizeof(MMRS));
        recomp_printf("Created zseq at address %p\n", &allMmrs[i].zseq);
        recomp_printf("==Data spans from %p to %p==\n", &allMmrs[i].zseq.data, &allMmrs[i].zseq.data[32767]);
    }

    recomp_printf("\nReading MMRS files into address %p\n", &allMmrs[1]);
    read_mmrs_files(allMmrs);
    recomp_printf("\nRead MMRS files into address %p!\n", &allMmrs);
    recomp_printf("Songname at entry 0: %s\n", allMmrs[0].songName);
    recomp_printf("Songname at entry 1: %s\n", allMmrs[1].songName);
    recomp_printf("Example zseq data at entry 1: ");
    
    for (int q = 0; q < 16; q++)
    {
        recomp_printf("%02x ", allMmrs[1].zseq.data[q]);
    }
    recomp_printf("...\n");

    recomp_printf("...\n");

    recomp_printf("\nWriting...\n");

    AudioTableEntry *mySeq;

for (int i = 0; i < numMmrs; i++)
    {
        mySeq = recomp_alloc(sizeof(AudioTableEntry));

        recomp_printf("\nSize is %d", allMmrs[i].zseq.size);

        mySeq->romAddr = (uintptr_t) &allMmrs[i].zseq.data[0];
        mySeq->size = allMmrs[i].zseq.size;
        mySeq->medium = MEDIUM_CART;
        mySeq->cachePolicy = CACHE_EITHER;
        mySeq->shortData1 = 0;
        mySeq->shortData2 = 0;
        mySeq->shortData3 = 0;

        recomp_printf("\nCreated audiotable thing for song %d: %s!\n", i, allMmrs[i].songName);
        recomp_printf("Size of data: %i\n", mySeq->size);
        recomp_printf("===Data spans from %p to %p==\n", mySeq->romAddr, mySeq->romAddr + mySeq->size);
        recomp_printf("AudioTableEntry data: ");
        for (int q = 0; q < 16; q++)
        {
            recomp_printf("%02x ", *((unsigned char*)mySeq->romAddr + q));
        }
        recomp_printf("...\n");

        AudioApi_AddSequence(mySeq);

        if (!strcmp(allMmrs[i].songName, "ULTRAKILL - The Fire Is Gone"))
        {
            AudioApi_ReplaceSequence(NA_BGM_FILE_SELECT, mySeq);
            recomp_printf("Replaced sequence successfully!\n");
            AudioApi_ReplaceSequenceFont(NA_BGM_FILE_SELECT, 0, allMmrs[i].zseq.bankNo);
            recomp_printf("Replaced sequence font successfully! (bank %x)\n", allMmrs[i].zseq.bankNo);
        }
        if (!strcmp(allMmrs[i].songName, "snd_ominous"))
        {
            AudioApi_ReplaceSequence(NA_BGM_GET_ITEM, mySeq);
            AudioApi_ReplaceSequenceFont(NA_BGM_GET_ITEM, 0, allMmrs[i].zseq.bankNo);
            AudioApi_ReplaceSequence(NA_BGM_GET_NEW_MASK, mySeq);
            AudioApi_ReplaceSequenceFont(NA_BGM_GET_NEW_MASK, 0, allMmrs[i].zseq.bankNo);
            AudioApi_ReplaceSequence(NA_BGM_GET_SMALL_ITEM, mySeq);
            AudioApi_ReplaceSequenceFont(NA_BGM_GET_SMALL_ITEM, 0, allMmrs[i].zseq.bankNo);
        }
    }
}

RECOMP_HOOK("AudioHeap_Init") void AudioHeap_Debug()
{
    // recomp_printf("\nCalled AudioHeap_Debug()\n");
    // recomp_printf("\n==SAMPLING FREQUENCY: %i==\n", gAudioSpecs[gAudioCtx.specId].samplingFreq);
}