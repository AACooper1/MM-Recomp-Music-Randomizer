#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "audio_api.c"
#include "seq_tests/idkwhatsongthisis.xxd"

#include <stdint.h>
#include <stdlib.h>

static uintptr_t sDevAddr = 0;
static u8* sRamAddr = NULL;
static size_t sSize = 0;
static s32 sMedium = 0;

static char *seq = NULL;

int read_seq()
{
    FILE *seqFile = fopen("seq_tests/24.zseq", "rb");
    size_t i, len;

    if (seq == NULL)
    {
        perror("Could not open seq_tests/24.zseq");
        return EXIT_FAILURE;
    }

    fseek(seqFile, 0, SEEK_END);
    len = ftell(seqFile);
    rewind(seqFile);
    seq = malloc(len + 1);
    if(NULL == seq)
    {
        exit(-1);
    }


    fread(seq, 1, len, seqFile);
    fclose(seqFile);
    
    seq[len] = '\0';
}

read_seq();

RECOMP_IMPORT(".", void audio_apo_replace_sequence(u32 id, void* modAddr, size_t size));

RECOMP_CALLBACK(".", audio_api_init) void my_audio_api_init() {
    audio_api_replace_sequence(NA_BGM_FILE_SELECT, seq, sizeof(seq));
}