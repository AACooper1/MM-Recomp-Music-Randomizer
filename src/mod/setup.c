#include "setup.h"

LoadingScreen loadingScreen;
char loading_title_text[20] = "Music Rando Loading\0";
char loading_title_ellipse[4] = ".\0\0\0";

RECOMP_HOOK("ConsoleLogo_Init") void get_game_state_x(GameState* thisx)
{
    gxState = thisx;
}

RECOMP_CALLBACK(".", music_rando_begin) void start()
{
    music_rando_update_db();
}

void music_rando_update_db()
{
    update_log_level();
    logger.debug("%s", "Mod-side logger OK!\n");
    logger.noheader.debug("%s", "Mod-side no-header logger OK!\n");

    logger.noheader.debug("Moving into setup gamestate!\n");
    gxState->main = music_rando_setup_main;
}

void music_rando_create_loading_screen()
{
    loadingScreen.context = recompui_create_context();
    recompui_open_context(loadingScreen.context);
    loadingScreen.root = recompui_context_root(loadingScreen.context);

    recompui_set_context_captures_input(loadingScreen.context, false);
    recompui_set_context_captures_mouse(loadingScreen.context, false);

    // Make the frame take up the entire window.
    recompui_set_position(loadingScreen.root, POSITION_ABSOLUTE);
    recompui_set_top(loadingScreen.root, 0, UNIT_DP);
    recompui_set_right(loadingScreen.root, 0, UNIT_DP);
    recompui_set_bottom(loadingScreen.root, 0, UNIT_DP);
    recompui_set_left(loadingScreen.root, 0, UNIT_DP);
    recompui_set_width_auto(loadingScreen.root);
    recompui_set_height_auto(loadingScreen.root);

    loadingScreen.bg_color.r = loadingScreen.bg_color.g = loadingScreen.bg_color.b = loadingScreen.bg_color.a = 0;
    recompui_set_background_color(loadingScreen.root, &loadingScreen.bg_color);

    recompui_set_flex_direction(loadingScreen.root, FLEX_DIRECTION_COLUMN);
    recompui_set_justify_content(loadingScreen.root, JUSTIFY_CONTENT_CENTER); // Main flex axis, i.e., horizontal centering
    recompui_set_align_items(loadingScreen.root, ALIGN_ITEMS_CENTER); // Cross flex axis, i.e., vertical centering

    loadingScreen.container = recompui_create_element(loadingScreen.context, loadingScreen.root);

    recompui_set_flex_grow(loadingScreen.container, 0.0f); // Do not scale container based on window size
    recompui_set_flex_shrink(loadingScreen.container, 0.0f);
    recompui_set_width(loadingScreen.container, 75.0f, UNIT_PERCENT); // Do not scale container based on child elements
    recompui_set_height(loadingScreen.container, 20.0f, UNIT_PERCENT);
    recompui_set_justify_content(loadingScreen.container, JUSTIFY_CONTENT_CENTER);

    recompui_set_display(loadingScreen.container, DISPLAY_BLOCK); // Display as own row
    recompui_set_padding(loadingScreen.container, 16.0f, UNIT_DP); // Padding outside container
    recompui_set_align_items(loadingScreen.container, ALIGN_ITEMS_STRETCH); // Stretch to width of container portion

    loadingScreen.black_color.r = loadingScreen.black_color.g = loadingScreen.black_color.b = loadingScreen.black_color.a =  0;

    // Create the header
    loadingScreen.header = recompui_create_element(loadingScreen.context, loadingScreen.container);
    recompui_set_display(loadingScreen.header, DISPLAY_FLEX);
    recompui_set_flex_direction(loadingScreen.header, FLEX_DIRECTION_ROW);
    recompui_set_height_auto(loadingScreen.header);
    loadingScreen.prellipsis_label = recompui_create_label(loadingScreen.context, loadingScreen.header, &loading_title_ellipse, LABELSTYLE_LARGE);
    recompui_set_color(loadingScreen.prellipsis_label, &loadingScreen.black_color);
    loadingScreen.header_label = recompui_create_label(loadingScreen.context, loadingScreen.header, &loading_title_text, LABELSTYLE_LARGE);
    loadingScreen.ellipsis_label = recompui_create_label(loadingScreen.context, loadingScreen.header, &loading_title_ellipse, LABELSTYLE_LARGE);
    loadingScreen.postllipsis_label = recompui_create_label(loadingScreen.context, loadingScreen.header, "...", LABELSTYLE_LARGE);
    recompui_set_color(loadingScreen.postllipsis_label, &loadingScreen.black_color);
    recompui_set_justify_content(loadingScreen.header, JUSTIFY_CONTENT_CENTER);
    recompui_set_text_align(loadingScreen.header, TEXT_ALIGN_CENTER);

    // Create the body
    loadingScreen.body = recompui_create_element(loadingScreen.context, loadingScreen.container);
    recompui_set_height_auto(loadingScreen.body);
    recompui_set_width_auto(loadingScreen.body);
    recompui_set_padding(loadingScreen.body, 16.0f, UNIT_DP);
    recompui_set_padding_top(loadingScreen.body, 32.0f, UNIT_DP);
    recompui_set_flex_direction(loadingScreen.body, FLEX_DIRECTION_COLUMN);
    recompui_set_justify_content(loadingScreen.body, JUSTIFY_CONTENT_FLEX_START);
    recompui_set_text_align(loadingScreen.body, TEXT_ALIGN_CENTER);

    loadingScreen.body_label = recompui_create_label(loadingScreen.context, loadingScreen.body, "Creating Thread", LABELSTYLE_NORMAL);

    recompui_close_context(loadingScreen.context);

    loadingScreen.ready = true;
}

int wait_cycles_passed = 0;

void music_rando_setup_main()
{
    char jobMsg[JOB_MSG_BUFFER_SIZE] = {0};

    int ellipsis_dots = (wait_cycles_passed / 50) % 3;

    switch (ellipsis_dots)
    {
        case 0:
            loading_title_ellipse[1] = loading_title_ellipse[2] = ' ';
            break;
        case 2:
            loading_title_ellipse[2] = '.';
        case 1:
            loading_title_ellipse[1] = '.';
            break;
    }

    switch (jobState)
    {
        case UNSTARTED:
            unsigned char* modPath = recomp_get_mod_folder_path();
            dbJobId = prepare_database(modPath);
            recomp_free(modPath);

            if (!loadingScreen.ready)
            {
                music_rando_create_loading_screen();
            }
            if (!loadingScreen.shown)
            {
                recompui_show_context(loadingScreen.context);
                loadingScreen.shown = true;
            }

            jobState = music_rando_poll_thread(dbJobId, jobMsg);
            break;
        case RUNNING:
            jobState = music_rando_poll_thread(dbJobId, jobMsg);

            recompui_open_context(loadingScreen.context);
            // Reset after errors
            RecompuiColor white = {255, 255, 255, 255};
            recompui_set_color(loadingScreen.header_label, &white);
            recompui_set_text(loadingScreen.header_label, &loading_title_text);

            recompui_set_text(loadingScreen.body_label, jobMsg);
            recompui_set_text(loadingScreen.ellipsis_label, loading_title_ellipse);
            recompui_set_text(loadingScreen.prellipsis_label, loading_title_ellipse);

            recompui_close_context(loadingScreen.context);

            break;
        case DONE:
            if (loadingScreen.shown)
            {
                recompui_hide_context(loadingScreen.context);
                loadingScreen.shown = false;
            }
            music_rando_cleanup_thread(dbJobId);
            init_startup_menu();
            break;
        case ERROR:
            logger.error("Job returned error!\n");

            jobState = music_rando_poll_thread(dbJobId, jobMsg);

            recompui_open_context(loadingScreen.context);
            recompui_set_text(loadingScreen.body_label, jobMsg);
            recompui_close_context(loadingScreen.context);

            jobState = send_thread_msg(dbJobId, WAIT_CONTINUE);

            break;
        case WAIT_CONTINUE:
            if (CHECK_BTN_ANY(CONTROLLER1(gxState)->press.button, (0xFFBF /* Any button */)))
            {
                jobState = send_thread_msg(dbJobId, CONTINUE);
                break;
            }
            jobState = music_rando_poll_thread(dbJobId, jobMsg);

            recompui_open_context(loadingScreen.context);
            RecompuiColor red = {255, 0, 0, 255};
            recompui_set_color(loadingScreen.header_label, &red);
            recompui_set_text(loadingScreen.header_label, "Music Rando ERROR\n(Press any button to continue)");
            recompui_set_text(loadingScreen.body_label, jobMsg);
            recompui_set_text(loadingScreen.ellipsis_label, "");
            recompui_close_context(loadingScreen.context);
            break;
        case CONTINUE:
            jobState = music_rando_poll_thread(dbJobId, jobMsg);
            break;
        case FATAL:
            logger.error("Job returned fatal error!\n");
            init_startup_menu();
            break;
    }

    func_8012CF0C(gxState->gfxCtx, true, false, 0, 0, 0);

    wait_cycles_passed++;
}