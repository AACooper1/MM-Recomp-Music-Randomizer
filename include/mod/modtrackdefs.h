#ifndef __cplusplus
    #include "modding.h"
    #include "global.h"
    #include "recomputils.h"
    #include "recompconfig.h"

    #include "stdbool.h"
#endif

typedef enum cTrackType_t
{
    UNKNOWN,
    VANILLA,
    MMRS,
    OOTRS,
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
    char pad[4] ;
    #endif
} cSequence;

typedef struct cBank_t
{
    int id;
    char header[8];
    u32 size;
    char* data;
    #ifndef __cplusplus 
    char pad[4] ;
    #endif
} cBank;

typedef struct cSound_t
{
    int id;
    u32 size;
    char* data;
    #ifndef __cplusplus 
    char pad[4] ;
    #endif
    unsigned int sampleAddr;
} cSound;

typedef struct cTrack_t
{
    cTrackType type;
    char name[256];
    char slotName[256];
    int bankNo;
    int tableIdx;

    int hasSeq;
    int hasBank;
    int numSounds;

    cFormMask formmask;
    cSequence seq;
    cBank bank;
    cSound sounds[64];
} cTrack;