// C++ libraries
#ifdef __cplusplus
    #include <cstddef>
    #include <string>
    #include <cstdint>
    #include "lib_recomp.hpp"
#endif

// C libraries 
#ifndef __cplusplus
    #include "stdbool.h"
#endif

// Constants
#define MAX_ZSEQ_SIZE 32768     // 32 KiB
#define MAX_ZBANK_SIZE 32768    // 32 KiB
#define MAX_ZSOUND_SIZE 131072  // 128 KiB
#define MAX_MMRS_SIZE 1048576   // 1 MiB

// Enums
typedef enum playState_t {
    FIERCE_DEITY = 1 << 0x0,
    GORON = 1 << 0x1,
    ZORA = 1 << 0x2,
    DEKU = 1 << 0x3,
    HUMAN = 1 << 0x4,
    OUTDOORS = 1 << 0x5,
    INDOORS = 1 << 0x6,
    CAVE = 1 << 0x7,
    EPONA = 1 << 0x8,
    SWIM = 1 << 0x9,
    SPIKES = 1 << 0xA,
    COMBAT = 1 << 0xB,
    CRITICAL_HEALTH = 1 << 0xC,
} playState;

typedef enum trackType_t {
    MMRS = 0
} trackType;


// Structs
typedef struct Track_t {
    int id;
    trackType type;
    char name[256];
    int bankNo;
} Track;

typedef struct Zseq_t {
    int id;
    int size;
    unsigned char data[MAX_ZSEQ_SIZE];
} Zseq;

typedef struct Zbank_t{
    int id;
    int size;
    unsigned char header[8];
    unsigned char data[MAX_ZBANK_SIZE];
} Zbank;


typedef struct Zsound_t {
    int trackId;
    char label[256];
    int size;
    u32 sampleAddr;
    unsigned char data[MAX_ZSOUND_SIZE];
} Zsound;

typedef struct MMRS_t {
    int trackId;
    bool categories[256];
    u16 formmask[20];
} MMRS;
