#ifdef __cplusplus
    #include <cstddef>
    #include <vector>
    #include <string>
    #include <unordered_map>

    #include <filesystem>
    #include <stdbool.h>

    #include <unistd.h>
    #include <iostream>
    #include <string>
    #include <cstring>
    #include "lib_recomp.hpp"
    #include "miniz.h"
    #include "util.h"
    #include "sqlite3.h"

    namespace fs = std::filesystem;
    extern sqlite3 *db;
#endif

#ifndef __cplusplus
    #include "stdbool.h"
#endif

#define MAX_ZSEQ_SIZE 32768     // 32 KiB
#define MAX_ZBANK_SIZE 32768    // 32 KiB
#define MAX_ZSOUND_SIZE 131072  // 128 KiB
#define MAX_MMRS_SIZE 1048576   // 1 MiB

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

typedef struct Zseq_t {
    int size;
    unsigned char data[MAX_ZSEQ_SIZE];
} Zseq;

typedef struct Zbank_t{
    int bankId;
    int bankSize;
    unsigned char bankData[MAX_ZBANK_SIZE];
    int metaSize;
    unsigned char metaData[MAX_ZBANK_SIZE];
} Zbank;


typedef struct Zsound_t {
    int size;
    u32 sampleAddr;
    unsigned char data[MAX_ZSOUND_SIZE];
} Zsound;

typedef struct MMRS_t {
    int id;
    char songName[256];
    bool categories[256];
    int bankNo;
    u16 formmask[20];
    int zseqId;
    int bankInfoId;
} MMRS;

// #ifdef __cplusplus
//     extern "C" {
// #endif
// #ifdef __cplusplus
//     }
// #endif