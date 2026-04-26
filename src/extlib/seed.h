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
        Seed(long long seed, std::shared_ptr<Database> db, fs::path savePath, bool use_custom, bool use_vanilla, bool use_ootrs);
        
        long long seed;

        std::shared_ptr<Database> db;
        std::map<int, std::shared_ptr<Track>> tracks;
        std::set<int> songforceTracks;

        std::map<int, std::shared_ptr<Track>> randomized;

        void randomize();
        void randomize_slot(int i);
        std::vector<int> get_available_tracks(SongSlotID slotId);
        std::string get_slot_name(SongSlotID slotId);

        void apply_songtest();
        void apply_songforce();

        int save_seed();
        int load_seed(fs::path savePath);

    private:
        void populate_track_table();
        void prepare_song_slots();
        void link_slots_to_vanilla();
        void prepare_tracks();
        void clear_track_availability(int trackId);

        Category categories[0x200];

        bool use_custom;
        bool use_vanilla;
        bool use_ootrs;
        bool do_not_randomize;

        fs::path savePath;

        static std::array<std::shared_ptr<Track>, 0x80> vanillaTracks;


        static std::array<SongSlot, 0x80> songSlots;
        
};


#endif