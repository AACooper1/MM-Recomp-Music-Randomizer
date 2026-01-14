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
    MMRS,
    OOTRS,
    STREAMED,
    UNKNOWN
};

class Track
{
    public:
        Track(fs::path path);

        bool read_from_file();
        int databaseIndex;
        long long int timestamp;
        std::string name;
        std::unique_ptr<std::vector<char>> categories;
        unsigned short bankNo = 0;
        TrackType type;
        FormMask formmask;
        fs::path path;

        std::shared_ptr<Sequence> sequence;
        std::shared_ptr<Bank> bank;
        std::vector<std::shared_ptr<Sound>> sounds;

    private:
        bool read_from_mmrs();
        bool read_from_ootrs() {return false;};
        bool read_from_streamed() {return false;};

        void parse_categories(std::shared_ptr<std::vector<char>> filebuffer);

        
        
        mz_zip_archive archive;
};

#endif