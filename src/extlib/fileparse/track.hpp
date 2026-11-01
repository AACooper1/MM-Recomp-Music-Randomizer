#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <filesystem>
#include <chrono>

#include "common.hpp"

#include "audiofile.hpp"
#include "formmask.hpp"
#include "image.hpp"

#include "lib/miniz.h"

#include "seed/category.hpp"
#include "ootrs/ootrs.hpp"
#include "fileparse/config.hpp"

namespace fs = std::filesystem;

enum class TrackType
{
    UNKNOWN,
    VANILLA,
    MMRS,
    OOTRS,
    ZSEQ,
    STREAMED
};

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

            this->config = std::make_shared<Config>(id);
            this->config->parse_categories(categories);

            type = TrackType::VANILLA;
        }
        Track();

        bool read_from_file();
        bool is_fanfare() 
            {return ((*config->categories)[FANFARE] || (*config->categories)[AREA_CLEAR] || (*config->categories)[GAME_OVER] || _is_fanfare); }
        
        bool fix_oot_custom_bank(OoTAudioHandler* ootAudioHandler);

        int id = 0;
        int databaseIndex = 0;
        int seedIdx = 0;
        bool is_prepared = false;

        fs::path path;
        long long int timestamp;
        std::string name;
        unsigned short bankNo = 0;
        TrackType type;
        FormMask formmask;

        u32 seqId = 0;
        std::shared_ptr<Sequence> sequence = nullptr;

        int bankId = 0;
        std::shared_ptr<Bank> bank = nullptr;

        std::vector<int> soundIds;
        std::vector<std::shared_ptr<Sound>> sounds;

        int imageId = 0;
        std::shared_ptr<Image> image = nullptr;

        std::shared_ptr<Config> config;

    private:
        bool read_from_zseq();
        bool read_from_mmrs();
        bool read_from_ootrs();
        bool read_from_streamed() {return false;};

        bool _is_fanfare;
        
        bool parse_meta(std::vector<char>& filebuffer);
        
        mz_zip_archive archive;
};

#endif