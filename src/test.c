// C Libraries

// MM Recomp Mod Requirements
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

// Required Mods
#include "magemods_audio_api.h"

// Data Files
INCBIN(sequence, "src/seq_tests/ULTRAKILL - The Fire is Gone.zseq");

RECOMP_IMPORT("magemods_audio_api", void AudioApi_ReplaceSequence(AudioTableEntry entry, s32 seqId));
RECOMP_IMPORT("magemods_audio_api", void AudioApi_SetSequenceFontId(s32 seqId, s32 fontNum, s32 fontId));

// Stolen from magemods, who didn't export this function
void print_bytes(void *ptr, int size)
{
    unsigned char *p = ptr;
    int i;
    for (i = 0; i < size; i++) {
        recomp_printf("%02X ", p[i]);
    }
    recomp_printf("\n");
}

RECOMP_CALLBACK("magemods_audio_api", AudioApi_onInit) void my_mod_on_init() {    
    bool debug=false;
    if(!debug) return;

    AudioTableEntry mySeq = {
        (uintptr_t) sequence,    // romAddr
        sequence_end - sequence, // size
        MEDIUM_CART,
        CACHE_EITHER,
        0, 0, 0,
    };

    AudioApi_ReplaceSequence(mySeq, NA_BGM_FILE_SELECT);
    AudioApi_SetSequenceFontId(NA_BGM_FILE_SELECT, 0, 36);
}