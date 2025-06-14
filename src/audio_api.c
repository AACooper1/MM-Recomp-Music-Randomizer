#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "debug_util.h"

extern DmaHandler sDmaHandler;

/* -------------------------------------------------------------------------------- */

RECOMP_DECLARE_EVENT(audio_api_init());

// We need more exports for replacing samples, etc

RECOMP_EXPORT void audio_api_replace_sequence(u32 id, u8 bankId, void* modAddr, size_t size) {
    // Audioseq index table and entry for the sequence to replace
    AudioTable* table = (AudioTable*)gSequenceTable;
    AudioTableEntry* entry = &table->entries[id];

    // Replace Audioseq index table entry
    entry->romAddr = (uintptr_t)modAddr - SEGMENT_ROM_START(Audioseq);
    entry->size = size; //ALIGN16(size);

    // Sequence-to-Bank table entry
    //      * Each entry is 2 sections:
    //          - First byte is the number of banks in the seq's soundfont
    //          - Following bytes are the list of those banks
    //        All sequences except Sequence 0 have 0x01 as first byte. Sequence 0 has 0x02, so is 3 bytes long.
    //      * Weird ass table header is 0x100 bytes long
    //      * + 1 because we are accessing the second byte in the entry
    //      * + 1 because Sequence 0 adds an extra
    u8* seqFontEntry = &gSequenceFontTable[(id * 2) + 0x102];

    // Replace Sequence-to-Bank table entry
    *seqFontEntry = bankId;
}

RECOMP_HOOK("AudioLoad_Init") void on_AudioLoad_Init() {
    audio_api_init();
}

/* -------------------------------------------------------------------------------- */

// Our custom function
s32 AudioApi_Dma_Mod(OSIoMesg* mesg, u32 priority, s32 direction, uintptr_t devAddr, void* ramAddr,
                     size_t size, OSMesgQueue* reqQueue, s32 medium, const char* dmaFuncType) {
      osSendMesg(reqQueue, NULL, OS_MESG_NOBLOCK);
      Lib_MemCpy(ramAddr, (void*)devAddr, size);
      return 0;
}

// The original function
s32 AudioApi_Dma_Rom(OSIoMesg* mesg, u32 priority, s32 direction, uintptr_t devAddr, void* ramAddr,
                     size_t size, OSMesgQueue* reqQueue, s32 medium, const char* dmaFuncType) {
    OSPiHandle* handle;

    if (gAudioCtx.resetTimer > 16) {
        return -1;
    }

    switch (medium) {
        case MEDIUM_CART:
            handle = gAudioCtx.cartHandle;
            break;

        case MEDIUM_DISK_DRIVE:
            // driveHandle is uninitialized and corresponds to stubbed-out disk drive support.
            // SM64 Shindou called osDriveRomInit here.
            handle = gAudioCtx.driveHandle;
            break;

        default:
            return 0;
    }

    if ((size % 0x10) != 0) {
        size = ALIGN16(size);
    }

    mesg->hdr.pri = priority;
    mesg->hdr.retQueue = reqQueue;
    mesg->dramAddr = ramAddr;
    mesg->devAddr = devAddr;
    mesg->size = size;
    handle->transferInfo.cmdType = 2;
    sDmaHandler(handle, mesg, direction);
    return 0;
}


RECOMP_PATCH s32 AudioLoad_Dma(OSIoMesg* mesg, u32 priority, s32 direction, uintptr_t devAddr, void* ramAddr,
                               size_t size, OSMesgQueue* reqQueue, s32 medium, const char* dmaFuncType) {
    if (IS_KSEG0(devAddr)) { // If it's in RAM
        return AudioApi_Dma_Mod(mesg, priority, direction, devAddr, ramAddr, size, reqQueue, medium, dmaFuncType);
    } else {
        return AudioApi_Dma_Rom(mesg, priority, direction, devAddr, ramAddr, size, reqQueue, medium, dmaFuncType);
    }
}