#ifndef FORMMASK_H
#define FORMMASK_H

#include <string>
#include <vector>
#include <util.h>
#include <logging.h>

extern Log logger;

class FormMask
{
    public:
        FormMask() 
        {
            std::fill_n(states, 16, 0xFFFF); 
            cumulativeStates = 0;
        }
        void parse(std::shared_ptr<std::vector<char>> filebuffer);

        unsigned short states[16];
        unsigned short cumulativeStates;
};

enum PlayState {
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
};

static std::string playStates[13] = 
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
    "CriticalHealth"
};

#endif