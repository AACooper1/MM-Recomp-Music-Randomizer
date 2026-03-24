#include "start_menu.h"

StartMenu startMenu;

static const char* randomization_options[] = {"On", "Off"};
static const char* track_source_options[] = {"Any", "Vanilla only", "Custom only"};
static const char* morning_song_options[] = {"Vanilla", "Randomized"};

StartMenu_Option music_rando_startup_options[OPTIONS_MAX] = 
{
    {"Randomization", randomization_options, 2, 0, &config.randomization_on},
    {"Use Tracks", track_source_options, 3, 0, &config.track_types},
    {"Morning Song", morning_song_options, 2, 1, &config.randomize_suns_song},
    
    {NULL, NULL, 0} // Sentinel
};

// Unlike rando, we don't want all menus to have this frame, so just define it here.
void music_rando_create_start_menu()
{
    startMenu.context = recompui_create_context();
    recompui_open_context(startMenu.context);
    startMenu.root = recompui_context_root(startMenu.context);

    recompui_set_context_captures_input(startMenu.context, true);
    recompui_set_context_captures_mouse(startMenu.context, true);

    // Make the frame take up the entire window.
    recompui_set_position(startMenu.root, POSITION_ABSOLUTE);
    recompui_set_top(startMenu.root, 0, UNIT_DP);
    recompui_set_right(startMenu.root, 0, UNIT_DP);
    recompui_set_bottom(startMenu.root, 0, UNIT_DP);
    recompui_set_left(startMenu.root, 0, UNIT_DP);
    recompui_set_width_auto(startMenu.root);
    recompui_set_height_auto(startMenu.root);

    startMenu.bg_color.r = startMenu.bg_color.g = startMenu.bg_color.b = startMenu.bg_color.a = 0;
    recompui_set_background_color(startMenu.root, &startMenu.bg_color);

    recompui_set_flex_direction(startMenu.root, FLEX_DIRECTION_COLUMN);
    recompui_set_justify_content(startMenu.root, JUSTIFY_CONTENT_CENTER); // Main flex axis, i.e., horizontal centering
    recompui_set_align_items(startMenu.root, ALIGN_ITEMS_CENTER); // Cross flex axis, i.e., vertical centering

    startMenu.container = recompui_create_element(startMenu.context, startMenu.root);
    
    recompui_set_flex_grow(startMenu.container, 0.0f); // Do not scale container based on window size
    recompui_set_flex_shrink(startMenu.container, 0.0f);
    recompui_set_width_auto(startMenu.container); // Automatically scale container based on child elements
    recompui_set_height_auto(startMenu.container);

    recompui_set_display(startMenu.container, DISPLAY_BLOCK); // Display as own row
    recompui_set_padding(startMenu.container, 16.0f, UNIT_DP); // Padding outside container
    recompui_set_align_items(startMenu.container, ALIGN_ITEMS_STRETCH); // Stretch to width of container portion

    startMenu.border_color.r = startMenu.border_color.g = startMenu.border_color.b = 255;
    startMenu.border_color.a = 0.7 * 255;
    recompui_set_border_width(startMenu.container, 5.1f, UNIT_DP);
    recompui_set_border_radius(startMenu.container, 16.0f, UNIT_DP);
    recompui_set_border_color(startMenu.container, &startMenu.border_color);

    startMenu.modal_color.r = 8;
    startMenu.modal_color.g = 7;
    startMenu.modal_color.b = 13;
    startMenu.modal_color.a = 0.9f * 255;
    recompui_set_background_color(startMenu.container, &startMenu.modal_color);

    // Create the header
    startMenu.header = recompui_create_element(startMenu.context, startMenu.container);
    recompui_set_height_auto(startMenu.header);
    recompui_create_label(startMenu.context, startMenu.header, "Music Randomization Options", LABELSTYLE_LARGE);

    startMenu.body = recompui_create_element(startMenu.context, startMenu.container);
    recompui_set_height_auto(startMenu.body);
    recompui_set_width_auto(startMenu.body);
    recompui_set_padding(startMenu.body, 16.0f, UNIT_DP);
    recompui_set_flex_direction(startMenu.body, FLEX_DIRECTION_COLUMN);

    for (int i = 0; i < ARRAY_COUNT(music_rando_startup_options); i++)
    {
        startMenu.options[i].option = &music_rando_startup_options[i];
        StartMenu_Option* this_option = startMenu.options[i].option;
        if (!this_option->name ) { break; }

        startMenu.options[i].container = recompui_create_element(startMenu.context, startMenu.body);
        recompui_set_flex_direction(startMenu.options[i].container, FLEX_DIRECTION_ROW);

        startMenu.options[i].label = recompui_create_label(startMenu.context, startMenu.options[i].container, startMenu.options[i].option->name, LABELSTYLE_NORMAL);
        startMenu.options[i].radio = recompui_create_labelradio(startMenu.context, startMenu.options[i].container, this_option->options, this_option->num_options);
        recompui_set_input_value_u32(startMenu.options[i].radio, this_option->default_value);
        
        recompui_set_padding_top(startMenu.options[i].label, 10.0f, UNIT_DP);
        recompui_set_padding_top(startMenu.options[i].radio, 10.0f, UNIT_DP);
        recompui_set_nav_auto(startMenu.options[i].radio, NAVDIRECTION_UP);
        recompui_set_nav_auto(startMenu.options[i].radio, NAVDIRECTION_DOWN);
    }

    startMenu.footer = recompui_create_element(startMenu.context, startMenu.container);
    recompui_set_height_auto(startMenu.footer);
    recompui_set_width_auto(startMenu.footer);
    recompui_set_padding(startMenu.footer, 16.0f, UNIT_DP);
    recompui_set_margin(startMenu.footer, 16.0f, UNIT_DP);
    recompui_set_flex_direction(startMenu.footer, FLEX_DIRECTION_COLUMN);

    startMenu.start_button = recompui_create_button(startMenu.context, startMenu.footer, "Start", BUTTONSTYLE_PRIMARY);
    recompui_set_nav_auto(startMenu.start_button, NAVDIRECTION_DOWN);
    recompui_register_callback(startMenu.start_button, start_button_pressed, NULL);

    recompui_close_context(startMenu.context);
    startMenu.ready = true;
}

void start_button_pressed(RecompuiResource resource, const RecompuiEventData* event, void* userdata)
{
    for (int i = 0; i < OPTIONS_MAX; i++)
    {
        if (!music_rando_startup_options[i].name)
        {
            break;
        }
        else
        {
            *music_rando_startup_options[i].config_value = recompui_get_input_value_u32(startMenu.options[i].radio);
        }
    }
    if (event->type == UI_EVENT_CLICK)
    {
        if (startMenu.shown)
        {
            recompui_hide_context(startMenu.context);
            startMenu.shown = false;
            logger.dev("Hode context!\n");
        }
        game_started = true;
    }
}

// We need to induce a gamestate similar to what rando does. Do this after ConsoleLogo_Init.
void music_rando_startup_menu_main()
{
    if (!startMenu.ready)
    {
        music_rando_create_start_menu();
    }
    if (!startMenu.shown)
    {
        recompui_show_context(startMenu.context);
        startMenu.shown = true;
    }
    if (game_started)
    {
        if (startMenu.shown)
        {
            recompui_hide_context(startMenu.context);
            startMenu.shown = false;
        }
        music_rando_update_db();
        gState->main = ConsoleLogo_Main;
    }

    func_8012CF0C(gState->gfxCtx, true, false, 0, 0, 0);
}

RECOMP_HOOK("ConsoleLogo_Init") void get_game_state(GameState* thisx)
{
    gState = thisx;
}

extern void Setup_Destroy(GameState* thisx);

RECOMP_HOOK_RETURN("ConsoleLogo_Init") void music_rando_startup_menu_init()
{
    gState->main = music_rando_startup_menu_main;
    gState->destroy = Setup_Destroy;
}