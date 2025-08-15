#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompui.h"

#include "mod_util.h"

// Enums
typedef enum Categories_t 
{
    CAT_FIELD,
    CAT_TOWN,
    CAT_DUNGEON,
    CAT_INDOORS,
    CAT_MINIGAME,
    CAT_ACTION,
    CAT_CALM,
    CAT_BOSS,
    CAT_FARFARE,
    CAT_GAME_OVER,
    CAT_AREA_CLEAR,
    CAT_SPECIAL
} Categories;

// Categories for vanilla sequences. Pulled from MMR
static Vector categorySequences[] = 
{
    // CAT_FIELD
    // CAT_TOWN
    // CAT_DUNGEON
    // CAT_INDOORS
    // CAT_MINIGAME
    // CAT_ACTION
    // CAT_CALM
    // CAT_BOSS
    // CAT_FANFARE
    // CAT_GAME_OVER
    // CAT_AREA_CLEAR
    // CAT_SPECIAL
};