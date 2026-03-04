#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompui.h"

#include "logging.h"
#include "modtrackdefs.h"

// I used Claude to write this and I'm not proud of that :(
#define FADE_OUT(a, t0, delay, fadetime)                                                                                  \
    (a) = (u8)(                                                                                                  \
        (osGetTime() < (t0) + (OS_USEC_TO_CYCLES((delay) * 1000 * 1000)))                                             \
            ? 0x1                                                                                                     \
        : (osGetTime() >= (t0) + OS_USEC_TO_CYCLES(((delay) * 1000 * 1000)) + OS_USEC_TO_CYCLES(((fadetime) * 1000 * 1000)))                      \
            ? 0xF                                                                                                 \
        : (u8)(0x1 + ((u32)(osGetTime() - (t0) - OS_USEC_TO_CYCLES((delay) * 1000 * 1000)) * 0xF) \
            / (u32)OS_USEC_TO_CYCLES((fadetime) * 1000 * 1000)) \
    )

typedef struct TitleDispCtx
{
    RecompuiContext context;
    RecompuiResource root;
    RecompuiResource container;
    RecompuiResource title_text;
    RecompuiColor bg_color;
    OSTime update_time;
    bool shown;
} TitleDispCtx;

void create_title_display_frame(char* songName);
void hide_title_display();
void show_title_display();

bool should_skip_song_title_display[0x7F];

#define SKIP_DISPLAY(i) should_skip_song_title_display[i] = true;

RECOMP_CALLBACK(".", music_rando_randomization_complete) void set_skip()
{
    SKIP_DISPLAY(NA_BGM_GENERAL_SFX);
    SKIP_DISPLAY(NA_BGM_AMBIENCE);
    SKIP_DISPLAY(NA_BGM_OCARINA_EPONA);
    SKIP_DISPLAY(NA_BGM_OCARINA_SUNS);
    SKIP_DISPLAY(NA_BGM_OCARINA_TIME);
    SKIP_DISPLAY(NA_BGM_OCARINA_STORM);
    SKIP_DISPLAY(NA_BGM_OCARINA_SOARING);
    SKIP_DISPLAY(NA_BGM_OCARINA_HEALING);
    SKIP_DISPLAY(NA_BGM_INVERTED_SONG_OF_TIME);
    SKIP_DISPLAY(NA_BGM_SONG_OF_DOUBLE_TIME);
    SKIP_DISPLAY(NA_BGM_SONATA_OF_AWAKENING);
    SKIP_DISPLAY(NA_BGM_GORON_LULLABY);
    SKIP_DISPLAY(NA_BGM_NEW_WAVE_BOSSA_NOVA);
    SKIP_DISPLAY(NA_BGM_ELEGY_OF_EMPTINESS);
    SKIP_DISPLAY(NA_BGM_OATH_TO_ORDER);
    SKIP_DISPLAY(NA_BGM_MIKAU_FINALE);
    SKIP_DISPLAY(NA_BGM_OCARINA_SONATA);
    SKIP_DISPLAY(NA_BGM_OCARINA_LULLABY);
    SKIP_DISPLAY(NA_BGM_OCARINA_NEW_WAVE);
    SKIP_DISPLAY(NA_BGM_OCARINA_ELEGY);
    SKIP_DISPLAY(NA_BGM_OCARINA_OATH);
    SKIP_DISPLAY(NA_BGM_OCARINA_LULLABY_INTRO);
    SKIP_DISPLAY(NA_BGM_OCARINA_LULLABY_INTRO_PTR);
    SKIP_DISPLAY(NA_BGM_SEQ_122);
}