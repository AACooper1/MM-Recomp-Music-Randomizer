#ifndef SEED_H
#define SEED_H

#include <array>
#include <random>
#include <set>

#include "lib_recomp.hpp"

#include "category.h"

#include "database.h"

class songSlot;
static auto rng = std::default_random_engine {};

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
    /* 0x10 */ AudioTableEntry entries[1]; // (dynamic size)
} AudioTable; // size >= 0x20


class SongSlot;
enum class SongSlotID;

class Seed
{
    public:
        Seed(long long seed, std::shared_ptr<Database> db, bool use_custom, bool use_vanilla) : 
            seed(seed), db(db), use_custom(use_custom), use_vanilla(use_vanilla) 
            { 
                if (!(use_custom || use_vanilla))
                {

                    throw("Tried to create custom music with custom and vanilla tracks both off?!");
                }

                for (int i = 0; i < 0x200; i++)
                {
                    categories[i] = Category(i);
                }

                populate_track_table();
                prepare_song_slots();
            }
        
        long long seed;

        std::shared_ptr<Database> db;
        std::map<int, std::shared_ptr<Track>> tracks;

        std::map<int, std::shared_ptr<Track>> randomized;

        void randomize();
        std::vector<int> get_available_tracks(SongSlotID slotId);

        AudioTable generate_audiotable();
    private:
        AudioTable* audioTableAddr;
        void populate_track_table();
        void prepare_song_slots();

        Category categories[0x200];

        bool use_custom;
        bool use_vanilla;

        static std::array<std::shared_ptr<Track>, 0x80> vanillaTracks;

        static std::array<SongSlot, 0x80> songSlots;
        
};

#endif