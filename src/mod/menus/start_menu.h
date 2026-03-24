#include "menu.h"
#include "logging.h"
extern Logger logger;

#define OPTIONS_MAX 64

typedef enum {
    OPTION_RADIO,
    OPTION_BOOL
} MusicRando_OptionType;

typedef struct StartMenu_Option_t {
    const char* name;
    const char** options;
    int num_options;
    int default_value;
    unsigned long* config_value;
} StartMenu_Option;

typedef struct MusicRando_Config_t {
    unsigned long randomization_on;
    unsigned long track_types;
    unsigned long randomize_suns_song;
} MusicRando_Config;

typedef struct StartMenu_Option_Element_t {
    RecompuiResource container;
    RecompuiResource label;
    RecompuiResource radio;

    StartMenu_Option* option;
} StartMenu_Option_Element;

typedef struct StartMenu_t {
    RecompuiContext context;
    RecompuiResource root;
    RecompuiResource container;
    
    RecompuiResource header;
    RecompuiResource header_label;

    RecompuiResource body;
    StartMenu_Option_Element options[OPTIONS_MAX];

    RecompuiResource footer;
    RecompuiResource start_button;

    RecompuiColor bg_color;
    RecompuiColor modal_color;
    RecompuiColor border_color;

    bool ready;
    bool shown;
} StartMenu;

GameState* gState;
bool game_started;

extern MusicRando_Config config;

// MM Functions
extern void ConsoleLogo_Main(GameState* thisx);
extern void ConsoleLogo_Destroy(GameState* thisx);

// Other music rando functions
extern void music_rando_update_db();

// start_menu.c functions

void music_rando_startup_menu_main();
void start_button_pressed(RecompuiResource resource, const RecompuiEventData* event, void* userdata);