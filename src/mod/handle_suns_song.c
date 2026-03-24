#include "handle_suns_song.h"

RECOMP_HOOK("AudioLoad_SyncInitSeqPlayer") void before_AudioLoad_SyncInitSeqPlayer(s32 playerIndex, s32 seqId, s32 arg2)
{
    _lastInitializedSeqPlayerIndex = playerIndex;
}

RECOMP_HOOK_RETURN("AudioLoad_SyncInitSeqPlayer") void after_AudioLoad_SyncInitSeqPlayer()
{
    SequencePlayer* seqPlayer = &gAudioCtx.seqPlayers[_lastInitializedSeqPlayerIndex];
    int seqId = seqPlayer->seqId;
    if (randomized[seqId].seq.id == 0x1D && seqId != 0x1D)
    {
        logger.debug("Sun's Song loaded on player %x. Handling...\n", seqPlayer->playerIndex);
        handle_morning_sequence(seqPlayer);
        logger.noheader.debug("Sun's Song handled!\n");
    }
    else if (seqId == 0x1D && randomized[seqId].seq.id != 0x1D)
    {
        logger.debug("Morning sequence is randomized. Handling...\n");
        add_control_flow_to_morning_slot(seqPlayer);
        logger.noheader.debug("Randomized Morning sequence handled!\n");
    }
}

void handle_morning_sequence(SequencePlayer* seqPlayer)
{
    logger.debug("Running handle_morning_sequence...\n");
    if (!sunsSongCopyAddr)
    {
        sunsSongCopyAddr = recomp_alloc(0x0610);
        Lib_MemCpy(sunsSongCopyAddr, seqPlayer->seqData, 0x0610);
        sunsSongCopyAddr[0x0D] = sunsSongCopyAddr[0x49] = 0x00;
        sunsSongCopyAddr[0x47] = 0xFF;
        logger.noheader.debug("Allocated 0x610 bytes at $0x%p for Sun's Song copy!", sunsSongCopyAddr);
        logger.noheader.dev(" New data:");
        logger.noheader.debug("\n");
        print_bytes(seqPlayer->scriptState.pc, 0x4A);
    }
    else
    {
        logger.noheader.debug("Sun's Song copy already prepared at $0x%p.\n", sunsSongCopyAddr);
    }
    seqPlayer->seqData = (u8*)sunsSongCopyAddr;
    seqPlayer->scriptState.pc = seqPlayer->seqData;
    logger.noheader.debug("Loaded modified Sun's Song into seqPlayer %x.\n", seqPlayer->playerIndex);
}

void add_control_flow_to_morning_slot(SequencePlayer* seqPlayer)
{
    logger.debug("Running add_control_flow_to_morning_slot...\n");
    if (!sunsSlotCopyAddr)
    {
        sunsSlotCopyAddr = recomp_alloc(gAudioCtx.sequenceTable->entries[seqPlayer->seqId].size + 0x100);
        Lib_MemCpy(sunsSlotCopyAddr, seqPlayer->seqData, gAudioCtx.sequenceTable->entries[seqPlayer->seqId].size);
        sunsSlotCopyControlFlowAddr = sunsSlotCopyAddr + gAudioCtx.sequenceTable->entries[seqPlayer->seqId].size + 0x80;
        
        // val = IO[0]; if val < 2 IO[0] = -1;
        sunsSlotCopyControlFlowAddr[0x00] = 0x80;
        
        // val -= 0x01;
        sunsSlotCopyControlFlowAddr[0x01] = 0xC8;
        sunsSlotCopyControlFlowAddr[0x02] = 0x01;
        
        // pc += 0x02 if val < 0;
        sunsSlotCopyControlFlowAddr[0x03] = 0xF2;
        sunsSlotCopyControlFlowAddr[0x04] = 0x02;

        // pc = &data + 0x00 (actual start of sequence)
        sunsSlotCopyControlFlowAddr[0x05] = 0xFB;
        sunsSlotCopyControlFlowAddr[0x06] = 0x00;
        sunsSlotCopyControlFlowAddr[0x07] = 0x00;

        // [In randomized morning seq now if 6 AM]
        // [After return from randomized morning slot]

        // val = IO[4]; if val < 2 IO[4] = -1;
        sunsSlotCopyControlFlowAddr[0x08] = 0x84;

        // val -= 0xEB;
        // (This gets val to 0x15, i.e., Clock Town Day 1, plus dayMinusOne)
        sunsSlotCopyControlFlowAddr[0x09] = 0xC8;
        sunsSlotCopyControlFlowAddr[0xA] = 0xEB;

        // data[0x10] = val;
        sunsSlotCopyControlFlowAddr[0x0B] = 0xC7;
        sunsSlotCopyControlFlowAddr[0x0C] = 0x00;

        sunsSlotCopyControlFlowAddr[0x0d] = (sunsSlotCopyControlFlowAddr - sunsSlotCopyAddr + 0x11) >> 8;
        sunsSlotCopyControlFlowAddr[0x0e] = (sunsSlotCopyControlFlowAddr - sunsSlotCopyAddr + 0x11) & 0xFF;

        // if (*++pc == 0xFF) AudioLoad_SyncInitSeqPlayer(playerId, *++pc, 0);
        sunsSlotCopyControlFlowAddr[0x0f] = 0xC4;
        sunsSlotCopyControlFlowAddr[0x10] = 0xFF;
        sunsSlotCopyControlFlowAddr[0x11] = 0x00;

        // return;
        sunsSlotCopyControlFlowAddr[0x12] = 0xFF;

        logger.noheader.debug("Prepared $0x%p for Morning Sequence!\n", sunsSlotCopyAddr);
    }
    else
    {
        logger.noheader.debug("Morning Sequence slot already prepared at $0x%p.\n", sunsSlotCopyAddr);
    }

    seqPlayer->seqData = (u8*)sunsSlotCopyAddr;
    seqPlayer->scriptState.pc = sunsSlotCopyControlFlowAddr;
    logger.noheader.debug("Loaded modified %s into seqPlayer %x.  Player has IO[0] value %x.", randomized[seqPlayer->seqId].name, seqPlayer->playerIndex, seqPlayer->seqScriptIO[0]);
    logger.noheader.dev(" New data:");
    logger.noheader.debug("\n");
    print_bytes(seqPlayer->scriptState.pc, 0x100);
    print_bytes(seqPlayer->seqData, 0x100);
}

RECOMP_HOOK_RETURN("AudioScript_SequencePlayerProcessSequence") void return_to_suns_slot()
{
    for (int i = 0; i < SEQ_PLAYER_MAX; i++)
    {
        if (gAudioCtx.seqPlayers[i].seqId == 0x1D && gAudioCtx.seqPlayers[i].finished)
        {
            logger.debug("Reached end of randomized Sun's Song slot. Playing Clock Town Day %x...\n", gSaveContext.save.day);
            AudioLoad_SyncInitSeqPlayer(i, 0x15 + gSaveContext.save.day - 1, 0);
            logger.noheader.debug("Success!\n");
        }
    }

    return;
}

// Prints live data read by seqPlayer playing Sun's Song. 
// Disabling but not deleting in case I need to use it in the future.

RECOMP_HOOK("AudioScript_ScriptReadU8") void print_the_thingy(SeqScriptState* state)
{
    if (state->pc - (u8*)sunsSlotCopyAddr < 0x1000 && state->pc - (u8*)sunsSlotCopyAddr > -0x1000)
    {
        logger.noheader.dev("%02x ", *state->pc);
    }
}

RECOMP_HOOK("AudioScript_ScriptReadS16") void print_the_thingy2(SeqScriptState* state)
{
    if (state->pc - (u8*)sunsSlotCopyAddr < 0x1000 && state->pc - (u8*)sunsSlotCopyAddr > -0x1000)
    {
        logger.noheader.dev("%04x ", *state->pc);
    }
}
