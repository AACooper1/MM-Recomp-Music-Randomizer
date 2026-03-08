#ifndef FORMMASK_H
#define FORMMASK_H

#include <string>
#include <vector>
#include <util.h>
#include "logging.hpp"

extern Log logger;

class FormMask
{
    public:
        FormMask() 
        {
            std::fill_n(states, 16, 0xFFFF); 
            cumulativeStates = 0;
        }
        void parse_file(std::vector<char>& filebuffer);

        bool is_default = true;

        unsigned short states[16];
        unsigned short cumulativeStates;
};

enum FormMaskState {
    FIERCE_DEITY = 1 << 0x0,
    GORON = 1 << 0x1,
    ZORA = 1 << 0x2,
    DEKU = 1 << 0x3,

    HUMAN = 1 << 0x4,
    OUTDOORS = 1 << 0x5,
    INDOORS = 1 << 0x6,
    CAVE = 1 << 0x7,

    EPONA = 1 << 0x8,
    SWIM = 1 << 0x9,
    SPIKES = 1 << 0xA,
    COMBAT = 1 << 0xB,
    
    CRITICAL_HEALTH = 1 << 0xC,
    DAY = 1 << 0xD,
    NIGHT = 1 << 0xE,
    FORMMASK_UNUSED = 1 << 0xF
};

static std::string playStates[15] = 
{
    "FierceDeity",
    "Goron",
    "Zora",
    "Deku",
    "Human",
    "Outdoors",
    "Indoors",
    "Cave",
    "Epona",
    "Swim",
    "SpikeRolling",
    "Combat",
    "CriticalHealth",
    "Day",
    "Night"
};

#endif