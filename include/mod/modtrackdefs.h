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
    /* 0x00 */ u16 states[16];
    /* 0x20 */ u16 cumulativeStates;
    /* 0x22 */ u16 pad[3]; /* Necessary because otherwise there is a misalignment LMAO */
} cFormMask; // size = 0x28

typedef struct cSequence_t
{
    /* 0x00 */ u32 id;
    /* 0x04 */ u32 size;
    /* 0x08 */ char* data;
        #ifndef __cplusplus 
    /* 0x0C */ u32 pad;
        #endif
} cSequence; // size = 0x10

typedef struct cBank_t
{
    /* 0x00 */ int id;
    /* 0x04 */ u32 size;
    /* 0x08 */ char header[8];
    /* 0x10 */ char* data;
        #ifndef __cplusplus 
    /* 0x14 */ u32 pad;
        #endif
} cBank; // size = 0x18

typedef struct cSound_t
{
    /* 0x00 */ int id;
    /* 0x04 */ u32 size;
    /* 0x08 */ u32 sampleAddr;
    /* 0x0C */ char* data;
        #ifndef __cplusplus 
    /* 0x10 */ u64 pad;  // Why did I make this u64?
        #endif
} cSound; // size = 0x18

typedef struct cImage_t
{
    /* 0x00 */ u32 id;
    /* 0x04 */ u32 size;
    /* 0x08 */ char* data;
        #ifndef __cplusplus 
    /* 0x0C */ u32 pad;
        #endif
} cImage; // size = 0x10

typedef struct cTrack_t
{
/* 0x000 */ cTrackType type;
/* 0x004 */ char name[0x100];
/* 0x104 */ char slotName[0x100];
/* 0x204 */ int bankNo;
/* 0x208 */ int slotIdx;

/* 0x20C */ int hasSeq;
/* 0x210 */ int hasBank;
/* 0x214 */ int numSounds;
/* 0x218 */ int hasImage;
/* 0x21C */ float volume;

/* 0x220 */ cFormMask formmask;
    // #ifndef __cplusplus 
    // u32 pad;
    // #endif
/* 0x248 */ cSequence seq;
/* 0x258 */ cBank bank;
/* 0x270 */ cSound sounds[MAX_SOUNDS];
/* 0x870 */ cImage image;
} cTrack; // size = 0x87C

#endif