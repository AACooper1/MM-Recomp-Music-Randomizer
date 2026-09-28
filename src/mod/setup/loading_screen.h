#include "util/globals.h"
#include "setup.h"

LoadingScreen loadingScreen;

char loading_title_text[20] = "Music Rando Loading\0";
char loading_title_ellipse[4] = ".\0\0\0";

typedef struct LoadingScreen_t {
    RecompuiContext context;
    RecompuiResource root;
    RecompuiResource container;

    RecompuiResource header;
    RecompuiResource header_label;
    RecompuiResource prellipsis_label;
    RecompuiResource ellipsis_label;
    RecompuiResource postllipsis_label;

    RecompuiResource body;
    RecompuiResource body_label;

    RecompuiResource error_options;
    RecompuiResource ignore;
    RecompuiResource ignore_button_label;
    RecompuiResource ignore_label;
    RecompuiResource remove;
    RecompuiResource remove_button_label;
    RecompuiResource remove_label;

    RecompuiColor bg_color;
    RecompuiColor black_color;
    RecompuiColor border_color;

    bool ready;
    bool shown;
} LoadingScreen;

void music_rando_loading_screen_main();
