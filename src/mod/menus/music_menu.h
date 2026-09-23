#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompui.h"

#include "logging.h"
#include "modtrackdefs.h"
#include "recompuiColors.h"

#include "menu_textures.h"

RECOMP_IMPORT(".", int reroll_slot(int jobId, int slotIdx));

#define ALBUM_ART_MAX_SIZE 64 * 64
#define TRACK_NAME_MAX_SIZE 256
#define SLOT_NAME_MAX_SIZE 256

#define BUTTON_WIDTH 32
#define BUTTON_HEIGHT 32

#define BUTTON_WIDTH_SMALL 16
#define BUTTON_HEIGHT_SMALL 16

#define TABLE_HEADER_SLOT "Slot"
#define TABLE_HEADER_TRACK "Track"
static const char headerText[2][16] = {TABLE_HEADER_SLOT, TABLE_HEADER_TRACK};

extern Logger logger;
extern int dbJobId;

RecompuiContext musicMenuContext;

typedef struct MusicMenu_Resource_t {
    RecompuiResource parent;
    RecompuiResource container;
} MusicMenu_Resource;

typedef struct MusicMenu_Icon_Button_t {
    MusicMenu_Resource _base;

    RecompuiResource icon;

    unsigned long (*callback)();
} MusicMenu_Icon_Button;



    typedef struct MusicMenu_Slot_Column_t {
        MusicMenu_Resource _base;
        MusicMenu_Resource cell;

        int slotIdx;
        RecompuiResource label;
        const char* name;

        RecompuiResource buttonsContainer;
        MusicMenu_Icon_Button settings;
        MusicMenu_Icon_Button selectTrack;
    } MusicMenu_Slot_Column;

        typedef struct MusicMenu_Album_Art_t {
            MusicMenu_Resource _base;

            RecompuiResource image;
            char* imageData; // 64*64
        } MusicMenu_Album_Art;

        typedef struct MusicMenu_Volume_t {
            MusicMenu_Resource _base;

            cTrack* track;

            RecompuiResource iconContainer;
            RecompuiResource icon;
            
            RecompuiResource slider;
        } MusicMenu_Volume;

    typedef struct MusicMenu_Track_Column_t {
        MusicMenu_Resource _base;
        MusicMenu_Resource cell;

        MusicMenu_Album_Art albumArt;

        RecompuiResource titleContainer;
        RecompuiResource label;
        const char* name;
        MusicMenu_Icon_Button editTitle;

        MusicMenu_Volume volume;

        RecompuiResource buttonsContainer;
        MusicMenu_Icon_Button rerollSlot;
        MusicMenu_Icon_Button trackSettings;
    } MusicMenu_Track_Column;

typedef struct MusicMenu_Track_Element_t {
    MusicMenu_Resource _base;
    MusicMenu_Resource row;

    cTrack* track;
    MusicMenu_Slot_Column slotCol;
    MusicMenu_Track_Column trackCol;
} MusicMenu_Track_Element;




typedef struct MusicMenu_Table_Header_t {
    MusicMenu_Resource _base;

    RecompuiResource labels[2];
    const char text[2][16];
} MusicMenu_Table_Header;

typedef struct MusicMenu_t {
    MusicMenu_Resource _base;

    MusicMenu_Table_Header tableHeader;

    RecompuiResource trackTable;
    MusicMenu_Track_Element** tracks;

    RecompuiResource nowPlaying;

    RecompuiColor bg_color;
    RecompuiColor modal_color;
    RecompuiColor border_color;

    bool ready;
    bool shown;
} MusicMenu;

MusicMenu musicMenu;

MusicMenu_Track_Element music_menu_create_row(cTrack* slot, RecompuiResource parent);
MusicMenu_Slot_Column music_menu_create_slot_column(cTrack* slot, RecompuiResource parent);
MusicMenu_Icon_Button music_menu_create_icon_button(RecompuiTextureHandle icon, RecompuiResource parent, unsigned long width, unsigned long height, unsigned long (*callback)());
MusicMenu_Volume music_menu_create_volume(RecompuiResource parent, cTrack* track);

#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

extern const u64 gMusicMenuNameTex[];

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

#define QUEST_MUSIC_MENU QUEST_SKULL_TOKEN


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

static CursorPointDirection sCursorPointLinksButWithMusicMenu[] = {
        { CURSOR_NONE_PAUSE, QUEST_REMAINS_TWINMOLD, QUEST_REMAINS_GYORG, QUEST_REMAINS_GOHT },     // QUEST_REMAINS_ODOLWA
        { QUEST_REMAINS_ODOLWA, QUEST_SHIELD, QUEST_REMAINS_TWINMOLD, CURSOR_TO_RIGHT },            // QUEST_REMAINS_GOHT
        { QUEST_REMAINS_ODOLWA, QUEST_SWORD, QUEST_HEART_PIECE, QUEST_REMAINS_TWINMOLD },           // QUEST_REMAINS_GYORG
        { QUEST_REMAINS_ODOLWA, QUEST_SHIELD, QUEST_REMAINS_GYORG, QUEST_REMAINS_GOHT },            // QUEST_REMAINS_TWINMOLD
        { QUEST_REMAINS_GOHT, QUEST_BOMB_BAG, QUEST_SWORD, CURSOR_TO_RIGHT },                       // QUEST_SHIELD
        { QUEST_REMAINS_GYORG, QUEST_QUIVER, QUEST_SONG_STORMS, QUEST_SHIELD },                     // QUEST_SWORD
        { QUEST_SONG_TIME, QUEST_MUSIC_MENU, CURSOR_TO_LEFT, QUEST_SONG_LULLABY },                      // QUEST_SONG_SONATA
        { QUEST_SONG_HEALING, QUEST_MUSIC_MENU, QUEST_SONG_SONATA, QUEST_SONG_BOSSA_NOVA },             // QUEST_SONG_LULLABY
        { QUEST_SONG_EPONA, QUEST_MUSIC_MENU, QUEST_SONG_LULLABY, QUEST_SONG_ELEGY },                   // QUEST_SONG_BOSSA_NOVA
        { QUEST_SONG_SOARING, QUEST_MUSIC_MENU, QUEST_SONG_BOSSA_NOVA, QUEST_SONG_OATH },              // QUEST_SONG_ELEGY
        { QUEST_SONG_STORMS, QUEST_MUSIC_MENU, QUEST_SONG_ELEGY, QUEST_QUIVER },                       // QUEST_SONG_OATH
        /*Mod slop*/ { QUEST_SONG_BOSSA_NOVA, CURSOR_NONE_PAUSE, QUEST_MUSIC_MENU, CURSOR_NONE_PAUSE },     // !!CURSOR_NONE_PAUSE!!
        { QUEST_BOMBERS_NOTEBOOK, QUEST_SONG_SONATA, CURSOR_TO_LEFT, QUEST_SONG_HEALING },          // QUEST_SONG_TIME
        { QUEST_BOMBERS_NOTEBOOK, QUEST_SONG_LULLABY, QUEST_SONG_TIME, QUEST_SONG_EPONA },          // QUEST_SONG_HEALING
        { QUEST_HEART_PIECE, QUEST_SONG_BOSSA_NOVA, QUEST_SONG_HEALING, QUEST_SONG_SOARING },       // QUEST_SONG_EPONA
        { QUEST_HEART_PIECE, QUEST_SONG_ELEGY, QUEST_SONG_EPONA, QUEST_SONG_STORMS },               // QUEST_SONG_SOARING
        { QUEST_HEART_PIECE, QUEST_SONG_OATH, QUEST_SONG_SOARING, QUEST_SWORD },                    // QUEST_SONG_STORMS
        /*Mod slop*/ { QUEST_SONG_OATH, CURSOR_NONE_PAUSE, CURSOR_NONE_PAUSE, QUEST_QUIVER },            // !!CURSOR_NONE_PAUSE!!
        { CURSOR_NONE_PAUSE, QUEST_SONG_TIME, CURSOR_TO_LEFT, QUEST_HEART_PIECE },                  // QUEST_BOMBERS_NOTEBOOK
        { QUEST_SWORD, CURSOR_NONE_PAUSE, QUEST_MUSIC_MENU, QUEST_BOMB_BAG },                          // QUEST_QUIVER
        { QUEST_SHIELD, CURSOR_NONE_PAUSE, QUEST_QUIVER, CURSOR_TO_RIGHT },                         // QUEST_BOMB_BAG
        /*Mod slop*/ { QUEST_SONG_LULLABY, CURSOR_NONE_PAUSE, CURSOR_TO_LEFT, QUEST_QUIVER },       // !!QUEST_MUSIC_MENU!!
        { CURSOR_NONE_PAUSE, QUEST_SONG_STORMS, QUEST_BOMBERS_NOTEBOOK, QUEST_REMAINS_GYORG }       // QUEST_HEART_PIECE
    };

typedef enum PauseState_Extended {
    /* 0x1C */ PAUSE_STATE_MUSICMENU_OPEN = 0x1C
};

typedef enum PauseMusicMenuState {
    /* 0x08 */ PAUSE_MUSICMENU_STATE_APPEARING = 0x08,
    /* 0x09 */ PAUSE_MUSICMENU_STATE_IDLE,
    /* 0x10 */ PAUSE_MUSICMENU_STATE_CLOSING
};