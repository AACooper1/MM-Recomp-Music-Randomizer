#ifndef SEED_H
#define SEED_H

#include <array>
#include <random>
#include <set>

#include "lib_recomp.hpp"

#include "category.h"

#include "database.h"

#define N_VANILLA_SEQS 0x7F
#define N_VANILLA_BANKS 0x28
#define N_VANILLA_SOUNDS 0xFF

class songSlot;
static auto rng = std::default_random_engine {};

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
        std::string get_slot_name(SongSlotID slotId);

    private:
        void populate_track_table();
        void prepare_song_slots();
        void link_slots_to_vanilla();
        void prepare_tracks();

        Category categories[0x200];

        bool use_custom;
        bool use_vanilla;

        static std::array<std::shared_ptr<Track>, 0x80> vanillaTracks;


        static std::array<SongSlot, 0x80> songSlots;
        
};


#endif