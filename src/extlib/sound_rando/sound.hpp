#ifndef SOUND_H
#define SOUND_H

#include "lib_recomp.hpp"

#define SND_NONE  0
#define SND_BEEP  1 << 0
#define SND_SHORT 1 << 1
#define SND_LONG  1 << 2
#define SND_SYS   1 << 3
#define SND_LOOP  1 << 8        // MMR doesn't do looping, but could eventually be done here?

#define NO_REPLACE false
#define REPLACE true

class SoundEffect
{
    public:
        SoundEffect() : id(0), name(""), effectTags(0), slotTags(0) {}
        SoundEffect(int id, std::string name, u16 effectTags, u16 slotTags) : id(id), name(name), effectTags(effectTags), slotTags(slotTags) {}
        int id;
        std::string name;
        
        u16 effectTags = 0;
        u16 slotTags = 0;
};

#endif