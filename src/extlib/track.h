#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <filesystem>
#include <chrono>
#include "audiofile.h"
#include "formmask.h"
#include "util.h"

#include "miniz.h"

namespace fs = std::filesystem;

enum class TrackType
{
    UNKNOWN,
    VANILLA,
    MMRS,
    OOTRS,
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

            this->categories = std::make_unique<std::vector<char>>();
            this->categories->resize(0x200);
            parse_categories(categories);

            type = TrackType::VANILLA;
        }
        Track();

        bool read_from_file();
        
        int databaseIndex;
        fs::path path;
        long long int timestamp;
        std::string name;
        std::unique_ptr<std::vector<char>> categories;
        unsigned short bankNo = 0;
        int id = 0;
        TrackType type;
        FormMask formmask;

        int seqId = 0;
        std::shared_ptr<Sequence> sequence = nullptr;

        int bankId = 0;
        std::shared_ptr<Bank> bank = nullptr;

        std::vector<int> soundIds;
        std::vector<std::shared_ptr<Sound>> sounds;

    private:
        bool read_from_mmrs();
        bool read_from_ootrs() {return false;};
        bool read_from_streamed() {return false;};

        void parse_categories(std::vector<char>& filebuffer);
        void parse_categories(const std::vector<int>& categories);

        
        
        mz_zip_archive archive;
};

#endif