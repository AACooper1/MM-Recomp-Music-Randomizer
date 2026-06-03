#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <filesystem>
#include <chrono>
#include "audiofile.h"
#include "formmask.hpp"
#include "util.h"

#include "miniz.h"

#include "category.h"
#include "ootrs.hpp"

namespace fs = std::filesystem;

enum class TrackType
{
    UNKNOWN,
    VANILLA,
    MMRS,
    OOTRS,
    STREAMED
};

extern std::unordered_map<std::string, std::vector<SongSlotID>> OoTBGMGroupsToCategories;
extern std::unordered_map<std::string, std::vector<SongSlotID>> OoTFanfareGroupsToCategories;

class Track
{
    public:
        Track(fs::path path);
        Track(std::string name) : Track() { this->name = name; }
        /* Use this overload for vanilla tracks */
        Track(std::string name, const std::vector<int>& categories, unsigned short bankNo, int id)
        {
            this->name = name;
            this->bankNo = bankNo;
            this->id = id;
            this->timestamp = 954494531;

            this->categories = std::make_unique<std::vector<char>>();
            this->categories->resize(0x200);
            parse_categories(categories);

            type = TrackType::VANILLA;
        }
        Track();

        bool read_from_file();
        bool is_fanfare() 
            {return ((*categories)[FANFARE] || (*categories)[AREA_CLEAR] || (*categories)[GAME_OVER] || _is_fanfare); }
        
        bool fix_oot_custom_bank(OoTAudioHandler* ootAudioHandler);

        int id = 0;
        int databaseIndex = 0;
        int seedIdx = 0;
        bool is_prepared = false;

        fs::path path;
        long long int timestamp;
        std::string name;
        std::unique_ptr<std::vector<char>> categories;
        unsigned short bankNo = 0;
        TrackType type;
        FormMask formmask;

        u32 seqId = 0;
        std::shared_ptr<Sequence> sequence = nullptr;

        int bankId = 0;
        std::shared_ptr<Bank> bank = nullptr;

        std::vector<int> soundIds;
        std::vector<std::shared_ptr<Sound>> sounds;

    private:
        bool read_from_mmrs();
        bool read_from_ootrs();
        bool read_from_streamed() {return false;};

        bool _is_fanfare;

        void parse_categories(std::vector<char>& filebuffer);
        void parse_categories(const std::vector<int>& categories);
        
        bool parse_meta(std::vector<char>& filebuffer);
        void parse_oot_categories(std::string categories_raw);
        
        mz_zip_archive archive;
};

#endif