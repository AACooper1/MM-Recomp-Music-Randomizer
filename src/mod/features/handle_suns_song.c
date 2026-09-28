#include "handle_suns_song.h"

RECOMP_HOOK("AudioLoad_SyncInitSeqPlayer") void before_AudioLoad_SyncInitSeqPlayer(s32 playerIndex, s32 seqId, s32 arg2)
{
    _lastInitializedSeqPlayerIndex = playerIndex;
}

RECOMP_HOOK_RETURN("AudioLoad_SyncInitSeqPlayer") void after_AudioLoad_SyncInitSeqPlayer()
{
    SequencePlayer* seqPlayer = &gAudioCtx.seqPlayers[_lastInitializedSeqPlayerIndex];
    int seqId = seqPlayer->seqId;
    if (randomized[seqId].seq.id == 0x1D && randomized[seqId].type == VANILLA && seqId != 0x1D)
    {
        logger.debug("Morning Sequence loaded on player %x. Handling...\n", seqPlayer->playerIndex);
        handle_morning_sequence(seqPlayer);
        logger.noheader.debug("Morning Sequence handled!\n");
    }
    else if (seqId == 0x1D && randomized[seqId].seq.id != 0x1D)
    {
        logger.debug("Morning Sequence slot is randomized (id %x). Handling...\n", randomized[seqId].seq.id);
        add_control_flow_to_morning_slot(seqPlayer);
        logger.noheader.debug("Randomized Morning Sequence slot handled!\n");
    }
}

void handle_morning_sequence(SequencePlayer* seqPlayer)
{
    logger.debug("Running handle_morning_sequence...\n");
    if (!sunsSongCopyAddr)
    {
        // Check the thing we're copying into is actually the morning sequence
        if (seqPlayer->seqData[0x0D] != 0x2D || seqPlayer->seqData[0x47] != 0xC4)
        {
            logger.debug("Sequence data did not match Morning Sequence data!\n");
        }

        sunsSongCopyAddr = recomp_alloc(0x0610);
        Lib_MemCpy(sunsSongCopyAddr, seqPlayer->seqData, 0x0610);
        sunsSongCopyAddr[0x0D] = sunsSongCopyAddr[0x49] = 0x00;
        sunsSongCopyAddr[0x47] = 0xFF;
        logger.noheader.debug("Allocated 0x610 bytes at $0x%p for Morning Sequence copy!", sunsSongCopyAddr);
        logger.noheader.dev(" New data:");
        logger.noheader.debug("\n");
        print_bytes(&logger, seqPlayer->scriptState.pc, 0x4A);
    }
    else
    {
        logger.noheader.debug("Morning Sequence copy already prepared at $0x%p.\n", sunsSongCopyAddr);
    }
    seqPlayer->seqData = (u8*)sunsSongCopyAddr;
    seqPlayer->scriptState.pc = seqPlayer->seqData;
    logger.noheader.debug("Loaded modified Morning Sequence into seqPlayer %x.\n", seqPlayer->playerIndex);
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
    print_bytes(&logger, seqPlayer->scriptState.pc, 0x100);
    print_bytes(&logger, seqPlayer->seqData, 0x100);
}

RECOMP_HOOK_RETURN("AudioScript_SequencePlayerProcessSequence") void return_to_suns_slot()
{
    for (int i = 0; i < SEQ_PLAYER_MAX; i++)
    {
        if (gAudioCtx.seqPlayers[i].seqId == 0x1D && gAudioCtx.seqPlayers[i].finished)
        {
            logger.debug("Reached end of randomized Morning Sequence slot. Playing Clock Town Day %x...\n", gSaveContext.save.day);
            AudioLoad_SyncInitSeqPlayer(i, 0x15 + gSaveContext.save.day - 1, 0);
            logger.noheader.debug("Success!\n");
        }
    }

    return;
}


/*
// Prints live data read by seqPlayer playing Sun's Song. 
// Disabling but not deleting in case I need to use it in the future.

bool should_print = false;

RECOMP_HOOK("AudioScript_SequencePlayerProcessSequence") void iadush(SequencePlayer* seqPlayer)
{
    if (seqPlayer->playerIndex || seqPlayer->seqId < NA_BGM_TERMINA_FIELD)
    {
        should_print = false;
    }
    else
    {
        should_print = true;
    }
}

RECOMP_HOOK("AudioScript_SequenceChannelProcessScript") void iasdush(SequenceChannel* channel)
{
    if (channel->seqPlayer->playerIndex || channel->seqPlayer->seqId < NA_BGM_TERMINA_FIELD)
    {
        should_print = false;
    }
    else
    {
        should_print = true;
    }
}

RECOMP_HOOK("AudioScript_ScriptReadU8") void print_the_thingy(SeqScriptState* state)
{
    if (!should_print)
    {
        return;
    }

    // // if ((state->pc - (u8*)sunsSlotCopyAddr < 0x1000 && state->pc - (u8*)sunsSlotCopyAddr > -0x1000))
    // if (*state->pc == 0xFD)
    // {
    //     logger.noheader.dev("Channels enabled: %x %x %x %x   %x %x %x %x   %x %x %x %x   %x %x %x %x \n", 
    //         gAudioCtx.seqPlayers[0].channels[15]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[14]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[13]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[12]->enabled,

    //         gAudioCtx.seqPlayers[0].channels[11]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[10]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[9]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[8]->enabled,

    //         gAudioCtx.seqPlayers[0].channels[7]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[6]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[5]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[4]->enabled,

    //         gAudioCtx.seqPlayers[0].channels[3]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[2]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[1]->enabled,
    //         gAudioCtx.seqPlayers[0].channels[0]->enabled
    //     );
    // }

    if (*(state->pc - 1) == 0xCE)
    {
        logger.noheader.dev("%02x ", *state->pc);
        logger.noheader.dev("   %02hu   ", (gAudioCtx.audioRandom >> 2 & 0xFF) % *state->pc);
    }
    else if (*state->pc == 0xFF)
    {
        return;
    }
    else
    {
        logger.noheader.dev("%02x ", *state->pc);
    }
}

RECOMP_HOOK("AudioScript_ScriptReadS16") void print_the_thingy2(SeqScriptState* state)
{
    if (should_print)
    {
        logger.noheader.dev("%02x %02x ", *state->pc, *(state->pc + 1));
    }
}

RECOMP_HOOK("AudioScript_ScriptReadCompressedU16") void print_the_thingy3(SeqScriptState* state)
{
    if (should_print)
    {
        u16 to_print = *(state->pc);
        if (to_print & 0x80)
        {
            to_print = (to_print << 8) & 0x7F00;
            to_print = *(state->pc + 1) | to_print;
        }
        logger.noheader.dev("%02x %02x ", to_print & 0x00FF, to_print & 0xFF00 >> 8);
        
        
        if (*(state->pc - 1) == 0xFD)
        {
            logger.noheader.dev("\n");
        }
    }
}


// RECOMP_HOOK("AudioScript_ScriptReadS16") void print_the_thingy2(SeqScriptState* state)
// {
//     if (state->pc - (u8*)sunsSlotCopyAddr < 0x1000 && state->pc - (u8*)sunsSlotCopyAddr > -0x1000)
//     {
//         logger.noheader.dev("%04x ", *state->pc);
//     }
// }

*/