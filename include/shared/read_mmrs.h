#ifdef __cplusplus
    #include <cstddef>
    #include <vector>
    #include <string>

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

#define MAX_DATA_SIZE 32768

typedef struct Zseq_t {
    int size;
    unsigned char data[MAX_DATA_SIZE];
} Zseq;


typedef struct Zbank_t{
    int bankId;
    int bankSize;
    unsigned char bankData[MAX_DATA_SIZE];
    int metaSize;
    unsigned char metaData[MAX_DATA_SIZE];
} Zbank;


typedef struct Zsound_t {
    int size;
    unsigned char data[MAX_DATA_SIZE];
} Zsound;

typedef struct MMRSFileType {
    int id;
    char songName[256];
    bool categories[256];
    int bankNo;
    int formmask;
    int zseqId;
    int bankInfoId;
} MMRS;

// #ifdef __cplusplus
//     extern "C" {
// #endif
// #ifdef __cplusplus
//     }
// #endif