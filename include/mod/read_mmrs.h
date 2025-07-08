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
    #include "libzippp.h"
    namespace fs = std::filesystem;
    namespace lz = libzippp;
#endif

#ifndef __cplusplus
    #include "stdbool.h"
#endif

typedef struct Zseq_t {
    int size;
    void* data;
    int bankNo;
    #ifdef __cplusplus
        bool formmask[16] = {0};
    #endif
    #ifndef __cplusplus
        bool formmask[16];
    #endif
} Zseq;


typedef struct Zbank_t{
    int bankSize;
    void* bankData;
    int MetaSize;
    void* MetaData;
} Zbank;


typedef struct Zsound_t {
    char *size;
    void* data;
} Zsound;


typedef struct MMRSFileType {
    char* songName;
    #ifdef __cplusplus
        bool categories[256] = {0};
    #endif
    #ifndef __cplusplus
        bool categories[256];
    #endif
    Zseq *zseq;
    Zbank *bankInfo;
    Zsound *soundInfo;
} MMRS;

#ifdef __cplusplus
    extern "C" {
#endif

MMRS* read_seq_directory();

#ifdef __cplusplus
    }
#endif