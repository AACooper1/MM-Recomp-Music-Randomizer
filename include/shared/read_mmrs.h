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

typedef struct Zseq_t {
    int size;
    unsigned char data[32768];
    int bankNo;
    bool formmask[16];
} Zseq;


typedef struct Zbank_t{
    int bankSize;
    unsigned char bankData[32768];
    int MetaSize;
    unsigned char metaData[32768];
} Zbank;


typedef struct Zsound_t {
    int size;
    unsigned char data[32768];
} Zsound;

typedef struct MMRSFileType {
    char songName[256];
    bool categories[256];
    Zseq zseq;
    Zbank bankInfo;
    Zsound soundInfo;
} MMRS;

#ifdef __cplusplus
    extern "C" {
#endif
#ifdef __cplusplus
    }
#endif