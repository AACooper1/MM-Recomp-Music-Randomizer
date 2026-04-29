#ifndef OOTRS_H
#define OOTRS_H

#include <filesystem>
#include <vector>
#include <unordered_map>

#include "miniz.h"

#include "lib_recomp.hpp"

#include "logging.hpp"
#include "songslot.h"

#define AUDIOTABLE_HEADER "Audiotable_index"
#define AUDIOTABLE "Audiotable"
#define BANKTABLE_HEADER "Audiobank_index"
#define BANKTABLE "Audiobank"

typedef enum SampleBankTableType {
    /* 0 */ SEQUENCE_TABLE,
    /* 1 */ FONT_TABLE,
    /* 2 */ SAMPLE_TABLE
} SampleBankTableType;

typedef struct AudioTableHeader {
    /* 0x00 */ s16 numEntries;
    /* 0x02 */ s16 unkMediumParam;
    /* 0x04 */ uintptr_t romAddr;
    /* 0x08 */ char pad[0x8];
} AudioTableHeader; // size = 0x10

typedef struct AudioTableEntry {
    /* 0x0 */ uintptr_t romAddr;
    /* 0x4 */ size_t size;
    /* 0x8 */ s8 medium;
    /* 0x9 */ s8 cachePolicy;
    /* 0xA */ s16 shortData1;
    /* 0xC */ s16 shortData2;
    /* 0xE */ s16 shortData3;
} AudioTableEntry; // size = 0x10

typedef struct AudioTable {
    /* 0x00 */ AudioTableHeader header;
    /* 0x10 */ AudioTableEntry *entries; // (dynamic size)
} AudioTable; // size >= 0x20

class OoTAudioBin
{
    public:
        OoTAudioBin(fs::path path);

        std::string expected_files[4] = { AUDIOTABLE_HEADER, AUDIOTABLE, BANKTABLE_HEADER, BANKTABLE};

        std::vector<char>* soundTableHeader;
        std::vector<char>* soundTable;

        std::vector<char>* bankTableHeader;
        std::vector<char>* bankTable;

        bool successfully_parsed = false;

    private:
        fs::path audiobinPath;
        mz_zip_archive archive;

        std::unordered_map<std::string, std::vector<char>> raw_files;
};

#endif