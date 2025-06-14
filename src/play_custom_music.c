#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "debug_util.h"

#include "../include/audio/load.h"
#include "seq_tests/ULTRAKILL - The Fire is Gone.xxd"

#include <stdint.h>
#include <stdlib.h>

extern uintptr_t AudioLoad_TrySyncLoadSampleBank(u32 sampleBankId, u32* outMedium, s32 noLoad);

static uintptr_t sDevAddr = 0;
static u8* sRamAddr = NULL;
static size_t sSize = 0;
static s32 sMedium = 0;

RECOMP_IMPORT(".", void audio_api_replace_sequence(u32 id, u32 bankId, void* modAddr, size_t size));

RECOMP_CALLBACK(".", audio_api_init) void replace_sequence() {
    audio_api_replace_sequence(NA_BGM_FILE_SELECT, __bank, __24_zseq, sizeof(__24_zseq));
}