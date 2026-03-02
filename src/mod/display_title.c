#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "logging.h"
#include "modtrackdefs.h"

cTrack* randomizedTable;
extern Logger logger;
extern u8 sStartSeqDisabled;

RECOMP_CALLBACK(".", music_rando_randomization_complete) void set_randomized_addr(cTrack* randomizedAddr)
{
    randomizedTable = randomizedAddr;
}

RECOMP_HOOK("AudioLoad_SyncLoadSeq") void the(s32 seqId)
{
    if (seqId >= 2 && logger.is_initialized)
    {
        logger.debug("I am in the AudioLoad_SyncLoadSeq function now. Name: %s (ID: %x)\n", randomizedTable[seqId].name, seqId);
    }
}