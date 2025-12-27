#ifndef TRACK_H
#define TRACK_H

#include <string>
#include "audiofile.h"

#include "miniz.h"

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

        Sequence* sequence;
        Bank* bank;
        std::vector<Sound>* sounds;

    private:
        bool read_from_mmrs();
        bool read_from_ootrs() {};
        bool read_from_streamed() {};

        fs::path path;
        
        mz_zip_archive archive;
        std::vector<char> filebuffer;
};

#endif