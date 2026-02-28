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

void print_title_to_console(u8 seqPlayerIndex, u8 seqId, u8 seqArgs, u16 fadeInDuration)
{
    if (!logger.is_initialized) return;
    if (seqId < 0x02 || seqId > 0x7f) return;
    if (seqPlayerIndex != SEQ_PLAYER_BGM_MAIN) return;
    logger.debug("Now playing on seqPlayer");
    logger.noheader.debug(" %i:", seqPlayerIndex);
    logger.noheader.debug(" %s", randomizedTable[seqId].name);
    logger.noheader.debug(" (id %x)\n", seqId);
    logger.noheader.debug("Args: %x, thoseotherones: %i, %i\n", seqArgs, gActiveSeqs[seqPlayerIndex].isWaitingForFonts, sStartSeqDisabled);
}

RECOMP_HOOK("AudioSeq_StartSequence") void the(u8 seqPlayerIndex, u8 seqId, u8 seqArgs, u16 fadeInDuration)
{
    if (seqPlayerIndex == 0)
    {
        recomp_printf("I am in the StartSequence function now. ID: %x\n", seqId);
        while (true) {}
    }
}

RECOMP_HOOK("AudioSeq_ProcessSeqCmd") void print_seq_cmd(u32 cmd)
{
    if (!logger.is_initialized) return;
    logger.dev("SeqCmd: ");
    u8 op = cmd >> 28;
    switch(op)
    {
        case 0x0: logger.noheader.dev("SEQCMD_OP_PLAY_SEQUENCE"); goto print_seq_name; break;
        case 0x1: logger.noheader.dev("SEQCMD_OP_STOP_SEQUENCE"); break;
        case 0x2: logger.noheader.dev("SEQCMD_OP_QUEUE_SEQUENCE"); break;
        case 0x3: logger.noheader.dev("SEQCMD_OP_UNQUEUE_SEQUENCE"); break;
        case 0x4: logger.noheader.dev("SEQCMD_OP_SET_SEQPLAYER_VOLUME"); break;
        case 0x5: logger.noheader.dev("SEQCMD_OP_SET_SEQPLAYER_FREQ"); break;
        case 0x6: logger.noheader.dev("SEQCMD_OP_SET_CHANNEL_VOLUME"); break;
        case 0x7: logger.noheader.dev("SEQCMD_OP_SET_SEQPLAYER_IO"); break;
        case 0x8: logger.noheader.dev("SEQCMD_OP_SET_CHANNEL_IO"); break;
        case 0x9: logger.noheader.dev("SEQCMD_OP_SET_CHANNEL_IO_DISABLE_MASK"); break;
        case 0xA: logger.noheader.dev("SEQCMD_OP_SET_CHANNEL_DISABLE_MASK"); break;
        case 0xB: logger.noheader.dev("SEQCMD_OP_TEMPO_CMD"); break;
        case 0xC: logger.noheader.dev("SEQCMD_OP_SETUP_CMD"); break;
        case 0xD: logger.noheader.dev("SEQCMD_OP_SET_CHANNEL_FREQ"); break;
        case 0xE: logger.noheader.dev("SEQCMD_OP_GLOBAL_CMD"); break;
        case 0xF: logger.noheader.dev("SEQCMD_OP_RESET_AUDIO_HEAP"); break;
        default: logger.noheader.dev("%x", cmd >>28); break;
    }

    print_seq_name:
        u8 seqPlayerIndex = (cmd & SEQCMD_SEQPLAYER_MASK) >> 24;
        u8 seqId = cmd & SEQCMD_SEQID_MASK;
        u8 seqArgs = (cmd & 0xFF00) >> 8;
        u16 fadeTimer = (cmd & 0xFF0000) >> 13;
        print_title_to_console(seqPlayerIndex, seqId, seqArgs, fadeTimer);
}