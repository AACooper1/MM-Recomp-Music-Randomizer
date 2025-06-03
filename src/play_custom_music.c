#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "seq_tests/idkwhatsongthisis.xxd"

#include <stdint.h>
#include <stdlib.h>

static uintptr_t sDevAddr = 0;
static u8* sRamAddr = NULL;
static size_t sSize = 0;
static s32 sMedium = 0;

RECOMP_IMPORT(".", void audio_api_replace_sequence(u32 id, void* modAddr, size_t size));

RECOMP_CALLBACK(".", audio_api_init) void my_audio_api_init() {
    audio_api_replace_sequence(NA_BGM_FILE_SELECT, __24_zseq, sizeof(__24_zseq));
}