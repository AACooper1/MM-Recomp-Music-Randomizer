#include "display_title.h"

TitleDispCtx titleDispCtx;

RECOMP_CALLBACK(".", music_rando_randomization_complete) void prepare_title_display(cTrack* randomizedAddr)
{
    randomizedTable = randomizedAddr;

    titleDispCtx.context = recompui_create_context();
    recompui_open_context(titleDispCtx.context);
    logger.dev("Opened titleDispCtx\n");
    recompui_set_context_captures_input(titleDispCtx.context, false);
    recompui_set_context_captures_mouse(titleDispCtx.context, false);

    titleDispCtx.root = recompui_context_root(titleDispCtx.context);
    titleDispCtx.container = recompui_create_element(titleDispCtx.context, titleDispCtx.root);
    titleDispCtx.title_text = recompui_create_label(titleDispCtx.context, titleDispCtx.container, "", LABELSTYLE_LARGE);

    titleDispCtx.bg_color.r = 0;
    titleDispCtx.bg_color.g = 0;
    titleDispCtx.bg_color.b = 0;
    titleDispCtx.bg_color.a = 0;
    
    recompui_set_position(titleDispCtx.root, POSITION_ABSOLUTE);
    recompui_set_top(titleDispCtx.root, 95, UNIT_PERCENT);
    recompui_set_right(titleDispCtx.root, 60, UNIT_PERCENT);
    recompui_set_bottom(titleDispCtx.root, 0, UNIT_PERCENT);
    recompui_set_left(titleDispCtx.root, 1.5, UNIT_PERCENT);

    recompui_set_background_color(titleDispCtx.root, &titleDispCtx.bg_color);
    
    recompui_close_context(titleDispCtx.context);
}

RECOMP_HOOK("AudioLoad_SyncLoadSeq") void load_song_name(s32 seqId)
{
    if (seqId >= 2 && titleDispCtx.context)
    {
        logger.debug("Now playing: %s (ID: %x)\n", randomizedTable[seqId].name, seqId);
        if(!should_skip_song_title_display[seqId])
        {
            create_title_display_frame(randomizedTable[seqId].name);
        }
    }
}

void create_title_display_frame(char* songName)
{
    recompui_open_context(titleDispCtx.context);
    recompui_set_text(titleDispCtx.title_text, songName);
    titleDispCtx.bg_color.a = 1;
    titleDispCtx.update_time = osGetTime();
    recompui_set_opacity(titleDispCtx.container, titleDispCtx.bg_color.a);
    recompui_close_context(titleDispCtx.context);
    show_title_display();
}

void hide_title_display()
{
    if (titleDispCtx.shown)
    {
        recompui_hide_context(titleDispCtx.context);
        titleDispCtx.shown = false;
    }
}

void show_title_display()
{
    if (!titleDispCtx.shown && recomp_get_config_u32("show_song_titles"))
    {
        logger.dev("Showing title display context.\n");
        recompui_show_context(titleDispCtx.context);
        titleDispCtx.shown = true;
    }
}

RECOMP_HOOK_RETURN("Audio_Update") void lower_title_display_opacity()
{
    if (!titleDispCtx.context) {return;}
    recompui_open_context(titleDispCtx.context);
    if (titleDispCtx.bg_color.a >= 0xF) 
    {
        hide_title_display(); 
        recompui_close_context(titleDispCtx.context); 
        return;
    }

    FADE_OUT(titleDispCtx.bg_color.a, titleDispCtx.update_time, 2, 1);
    recompui_set_opacity(titleDispCtx.container, titleDispCtx.bg_color.a);
    recompui_close_context(titleDispCtx.context);
}