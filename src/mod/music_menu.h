#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "mod_util.h"
#include "recompui.h"

#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

/*
    NOTES ON PAUSE MENU CODE:

    - void KaleidoScope_SetVertices() [z_kaleido_scope_NES.c: 1948] sets the pause menu state
        - Counterintuitively, if statements update page opposite (since only page + 2 neighbors are visible at once)
            - PAUSE_QUEST block updates SELECT ITEM page
            - PAUSE_MASK block updates MAP page
            - PAUSE_ITEM block updates QUEST page
            - PAUSE_MAP block updates MASKS page

    - So I think I want to RECOMP_HOOK_RETURN the KaleidoScope_SetVertices() function

    - I'm not sure I need to add a button, I could just repurpose the ocarina buttons.

    - For loop in PAUSE_ITEM block (QUEST page update):
        - enum marked wrong in h file, for some reason. QUAD_QUEST_MAX is 39, not 41.
        - j is the box number, k is the individual vertex of the box (or possible the rotating dot?)

    - Actually I think all of the above is gfx code specifically for the rotation and stuff so I think I don't need to deal with it right now

    - Okay so yeah I do need to add the cursors arrays.

    - Hold on. There are unused things there :eyes:
    - QUEST_SONG_SARIA, QUEST_SONG_SUN, QUEST_SKULL_TOKEN for rewind, skip, tracks buttons?
*/

#define QUEST_REWIND QUEST_SONG_SARIA
#define QUEST_FORWARD QUEST_SONG_SUN
#define QUEST_TRACKS QUEST_SKULL_TOKEN

#ifndef Z64_PAUSE_MENU_H
    typedef enum {
        /* -3 */ CURSOR_TO_LEFT = -3, // Cursor on the "scroll to left page" position
        /* -2 */ CURSOR_TO_RIGHT,     // Cursor on the "scroll to right page" position
        /* -1 */ CURSOR_NONE_PAUSE          // No position in that direction, cursor stays where it is
    } CursorPointNext;

    typedef struct {
        /* 0x0 */ s16 up;
        /* 0x2 */ s16 down;
        /* 0x4 */ s16 left;
        /* 0x6 */ s16 right;
    } CursorPointDirection; // size = 0x8
#endif

extern s32 KaleidoScope_UpdateQuestStatusPoint(PauseContext* pauseCtx, s16 point);
extern u8 sQuestSongPlayedOcarinaButtons[];
extern s16 sQuestSongPlayedOcarinaButtonsAlpha[];
extern s16 sQuestSongPlayedOcarinaButtonsNum;

typedef struct MusicMenu_t 
{
    RecompuiContext context;

    RecompuiResource root;
    RecompuiResource container;
    RecompuiResource header;
    RecompuiResource title;
    RecompuiResource body;
} MusicMenu;

MusicMenu musicMenu;

static CursorPointDirection sCursorPointLinksButWithMusicMenu[] = {
        { CURSOR_NONE_PAUSE, QUEST_REMAINS_TWINMOLD, QUEST_REMAINS_GYORG, QUEST_REMAINS_GOHT },     // QUEST_REMAINS_ODOLWA
        { QUEST_REMAINS_ODOLWA, QUEST_SHIELD, QUEST_REMAINS_TWINMOLD, CURSOR_TO_RIGHT },            // QUEST_REMAINS_GOHT
        { QUEST_REMAINS_ODOLWA, QUEST_SWORD, QUEST_HEART_PIECE, QUEST_REMAINS_TWINMOLD },           // QUEST_REMAINS_GYORG
        { QUEST_REMAINS_ODOLWA, QUEST_SHIELD, QUEST_REMAINS_GYORG, QUEST_REMAINS_GOHT },            // QUEST_REMAINS_TWINMOLD
        { QUEST_REMAINS_GOHT, QUEST_BOMB_BAG, QUEST_SWORD, CURSOR_TO_RIGHT },                       // QUEST_SHIELD
        { QUEST_REMAINS_GYORG, QUEST_QUIVER, QUEST_SONG_STORMS, QUEST_SHIELD },                     // QUEST_SWORD
        { QUEST_SONG_TIME, QUEST_TRACKS, CURSOR_TO_LEFT, QUEST_SONG_LULLABY },                      // QUEST_SONG_SONATA
        { QUEST_SONG_HEALING, QUEST_TRACKS, QUEST_SONG_SONATA, QUEST_SONG_BOSSA_NOVA },             // QUEST_SONG_LULLABY
        { QUEST_SONG_EPONA, QUEST_REWIND, QUEST_SONG_LULLABY, QUEST_SONG_ELEGY },                   // QUEST_SONG_BOSSA_NOVA
        { QUEST_SONG_SOARING, QUEST_FORWARD, QUEST_SONG_BOSSA_NOVA, QUEST_SONG_OATH },              // QUEST_SONG_ELEGY
        { QUEST_SONG_STORMS, QUEST_FORWARD, QUEST_SONG_ELEGY, QUEST_QUIVER },                       // QUEST_SONG_OATH
        /*Mod slop*/ { QUEST_SONG_BOSSA_NOVA, CURSOR_NONE_PAUSE, QUEST_TRACKS, QUEST_FORWARD },     // !!QUEST_REWIND!!
        { QUEST_BOMBERS_NOTEBOOK, QUEST_SONG_SONATA, CURSOR_TO_LEFT, QUEST_SONG_HEALING },          // QUEST_SONG_TIME
        { QUEST_BOMBERS_NOTEBOOK, QUEST_SONG_LULLABY, QUEST_SONG_TIME, QUEST_SONG_EPONA },          // QUEST_SONG_HEALING
        { QUEST_HEART_PIECE, QUEST_SONG_BOSSA_NOVA, QUEST_SONG_HEALING, QUEST_SONG_SOARING },       // QUEST_SONG_EPONA
        { QUEST_HEART_PIECE, QUEST_SONG_ELEGY, QUEST_SONG_EPONA, QUEST_SONG_STORMS },               // QUEST_SONG_SOARING
        { QUEST_HEART_PIECE, QUEST_SONG_OATH, QUEST_SONG_SOARING, QUEST_SWORD },                    // QUEST_SONG_STORMS
        /*Mod slop*/ { QUEST_SONG_OATH, CURSOR_NONE_PAUSE, QUEST_REWIND, QUEST_QUIVER },            // !!QUEST_FORWARD!!
        { CURSOR_NONE_PAUSE, QUEST_SONG_TIME, CURSOR_TO_LEFT, QUEST_HEART_PIECE },                  // QUEST_BOMBERS_NOTEBOOK
        { QUEST_SWORD, CURSOR_NONE_PAUSE, QUEST_FORWARD, QUEST_BOMB_BAG },                          // QUEST_QUIVER
        { QUEST_SHIELD, CURSOR_NONE_PAUSE, QUEST_QUIVER, CURSOR_TO_RIGHT },                         // QUEST_BOMB_BAG
        /*Mod slop*/ { QUEST_SONG_LULLABY, CURSOR_NONE_PAUSE, CURSOR_TO_LEFT, QUEST_REWIND },       // !!QUEST_TRACKS!!
        { CURSOR_NONE_PAUSE, QUEST_SONG_STORMS, QUEST_BOMBERS_NOTEBOOK, QUEST_REMAINS_GYORG },      // QUEST_HEART_PIECE
    };