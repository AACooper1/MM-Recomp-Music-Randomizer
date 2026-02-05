#ifndef CATEGORY_H
#define CATEGORY_H

#include <vector>
#include <algorithm>

enum CategoryID
{
    /*0x00*/ FIELD,
    /*0x01*/ TOWN,
    /*0x02*/ DUNGEON,
    /*0x03*/ BUILDING,
    /*0x04*/ MINIGAME,
    /*0x05*/ ACTION,
    /*0x06*/ CALM,
    /*0x07*/ BOSS,
    /*0x08*/ FANFARE,
    /*0x09*/ GAME_OVER,
    /*0x0A*/ AREA_CLEAR,
    /*0x10*/ SPECIAL = 0x10,
    /*0x157*/ FINAL_HOURS
};

struct Category
{
    Category() {}
    Category(int id) : id(CategoryID(id)) {}
    CategoryID id;
    std::vector<int> seqIDs;
};

#endif