#include "modtrackdefs.h"
#include "logging.h"

extern Logger logger;
extern cTrack randomized[0x80];

RECOMP_HOOK("Player_UpdateCamAndSeqModes") void print_seq_id(PlayState* play, Player* this)
{
    u16 seqId = gActiveSeqs[SEQ_PLAYER_BGM_MAIN].seqId;
    if (seqId < 2) { return; }
    logger.noheader.dev("SeqId: %x", seqId);
    logger.noheader.dev(" (%s)\n", randomized[seqId].name);
}

// AudioSeq_GetActiveSeqId() pulls from gActiveSeqs, which does not get update on AudioLoad_SyncLoadSeq()
// or AudioLoad_SyncInitSeqPlayer(). This means Clock Town is always at 0x1d (the morning sequence).
// so we update the value in gActiveSeqs. Does not seem to have ill effects?
RECOMP_HOOK("AudioLoad_SyncInitSeqPlayer") void update_activeseqs_seq_id(s32 playerIndex, s32 seqId, s32 arg2)
{
    gActiveSeqs[playerIndex].seqId = seqId;
    return;
}