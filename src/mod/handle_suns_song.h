#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "logging.h"
#include "modtrackdefs.h"
#include "audio_api/all.h"

extern Logger logger;
extern cTrack randomized[0x80];

s32 _lastInitializedSeqPlayerIndex;
char* sunsSongCopyAddr;
char* sunsSlotCopyAddr;
char* sunsSlotCopyControlFlowAddr;

void handle_morning_sequence(SequencePlayer* seqPlayer);
void add_control_flow_to_morning_slot(SequencePlayer* seqPlayer);