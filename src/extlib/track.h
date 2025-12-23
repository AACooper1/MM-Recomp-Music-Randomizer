#ifndef TRACK_H
#define TRACK_H

#include <string>
#include "audiofile.h"

class Track
{
    public:
        int id;
        std::string name;
        bool categories[0x200];
        int bankNo;

    private:
        Sequence* sequence;
        Bank* bank;
        std::vector<Sound>* sounds;
};

class MMRS : public Track
{
    public:
        FormMask formmask;
};

class OOTRS : public Track
{

};

class Streamed : public Track
{

};

#endif