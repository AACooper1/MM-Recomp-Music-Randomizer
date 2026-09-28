#include "bens_soundtrack_compat.h"

RECOMP_IMPORT("mm_bens_remastered_soundtrack", void BensSoundtrack_SetDisableChannelSwitching(int playerIndex, bool shouldDisable));
RECOMP_HOOK("AudioLoad_SyncInitSeqPlayer") void bens_soundtrack_disable_switching(s32 playerIndex, s32 seqId, s32 arg2)
{
    if (recomp_is_dependency_met("mm_bens_remastered_soundtrack") != DEPENDENCY_STATUS_FOUND) return;
    if (randomized[seqId].type == VANILLA)
    {
        BensSoundtrack_SetDisableChannelSwitching(playerIndex, false);
    }
    else
    {
        BensSoundtrack_SetDisableChannelSwitching(playerIndex, true);
    }
}
