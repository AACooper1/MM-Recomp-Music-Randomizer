#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "audio_api.c"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Code largely stolen from Garrett Cox from 2Ship on GitHub
// https://gist.github.com/garrettjoecox/fcad0e942fd12c484328f787b9db3f93

// Will be updated with original code later, but don't forget to credit him for the concept

static uintptr_t sDevAddr = 0;
static u8* sRamAddr = NULL;
static size_t sSize = 0;
static s32 sMedium = 0;

static char *seq;

RECOMP_HOOK("AudioLoad_SyncDma")
void* get_seqPtr(uintptr_t devAddr, u8* ramAddr, size_t size, s32 medium)
{
    sDevAddr = devAddr;
    sRamAddr = ramAddr;
    sSize = size;
    sMedium = medium;
}

void read_seq(char *seq)
{
    FILE *seqFile = fopen("seq_tests/24.zseq", "rb");
    char *buffer = NULL;
    size_t i, len;

    if (seq == NULL)
    {
        perror("Could not open seq_tests/24.zseq");
        return EXIT_FAILURE
    }

    fseq(seqFile, )

    fread(seq, 1, )
}


// For now, this just replaces the File Select sequence with the zseq provided by the file.
RECOMP_HOOK_RETURN("AudioLoad_SyncDma")
void injectSequence()
{
    if (sDevAddr == 0x62d80) { // File select sequence
        Lib_MemSet(sRamAddr, 0, sSize);
        Lib_MemCpy(sRamAddr, seq, sizeof(seq));
    }
}