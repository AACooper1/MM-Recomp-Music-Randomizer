#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "logging.h"
#include "modtrackdefs.h"
#include "audio_api/all.h"

#define SOUND_SHORT 1 << 0
#define SOUND_MED   1 << 1
#define SOUND_LONG  1 << 2

#define SOUND_REPLACABLE 1 << 15

typedef struct Sound_t
{
    int id;
    char name[256];
    u16 flags;

    u32 size;
    u32 sampleAddr;
} Sound;

typedef struct SoundSlot_t
{
    int id;
    char name[256];
    u16 flags;

    Sound sound;
} SoundSlot;

typedef struct SoundGroup_T
{
    int id;
    const char name[256];
    
    int numSlots;
    SoundSlot* slots;
} SoundGroup;