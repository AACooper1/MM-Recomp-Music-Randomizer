#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <filesystem>
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
        int id;
        std::string name;
        bool categories[0x200] = {false};
        unsigned short bankNo = 0;
        TrackType type;
        FormMask formmask;

        Sequence* sequence;
        Bank* bank;
        std::vector<Sound*> sounds;

    private:
        bool read_from_mmrs();
        bool read_from_ootrs() {return false;};
        bool read_from_streamed() {return false;};

        void parse_categories(std::shared_ptr<std::vector<char>> filebuffer);
        void parse_formmask(std::shared_ptr<std::vector<char>> filebuffer);

        fs::path path;
        
        mz_zip_archive archive;
};

#endif