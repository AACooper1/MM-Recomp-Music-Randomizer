#ifndef MODTRACKDEFS_H
#define MODTRACKDEFS_H

#ifndef __cplusplus
    #include "modding.h"
    #include "global.h"
    #include "recomputils.h"
    #include "recompconfig.h"

    #include "stdbool.h"
#endif

#define MAX_SOUNDS 64

typedef enum cTrackType_t
{
    UNKNOWN,
    VANILLA,
    MMRS,
    OOTRS,
    ZSEQ,
    STREAMED
} cTrackType;

typedef struct cFormMask_t
{
    u16 states[16];
    u16 cumulativeStates;
    u16 pad[3]; /* Necessary because otherwise there is a misalignment LMAO */
} cFormMask;

typedef struct cSequence_t
{
    u32 id;
    u32 size;
    char* data;
    #ifndef __cplusplus 
    u32 pad;
    #endif
} cSequence;

typedef struct cBank_t
{
    int id;
    u32 size;
    char header[8];
    char* data;
    #ifndef __cplusplus 
    u32 pad;
    #endif
} cBank;

typedef struct cSound_t
{
    int id;
    u32 size;
    u32 sampleAddr;
    char* data;
    #ifndef __cplusplus 
    u64 pad;
    #endif
} cSound;

typedef struct cTrack_t
{
    cTrackType type;
    char name[256];
    char slotName[256];
    int bankNo;
    int slotIdx;

    int hasSeq;
    // int hasFormMask;
    int hasBank;
    int numSounds;

    cFormMask formmask;
    cSequence seq;
    cBank bank;
    cSound sounds[MAX_SOUNDS];
} cTrack;

#endif