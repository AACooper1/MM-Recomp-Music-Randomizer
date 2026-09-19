#include "music_menu.h"

extern f32 sQuestStatusCursorsX[];
extern f32 sQuestStatusCursorsY[];

extern s16 sQuestVtxRectLeft[];
extern s16 sQuestVtxRectTop[];

extern s16 sQuestVtxWidths[];
extern s16 sQuestVtxHeights[];

extern u8 sAudioPauseState;

s32 seqId;
u16 seqArgs;

float volumeVals[NA_BGM_MAX] = {100.0f};

bool is_music_menu_open = false;
bool has_music_menu_been_opened = false;

RECOMP_IMPORT("magemods_audio_api", s32 AudioApi_GetActiveSeqId(u8 seqPlayerIndex));
RECOMP_IMPORT("magemods_audio_api", u16 AudioApi_GetActiveSeqArgs(u8 seqPlayerIndex));
RECOMP_IMPORT("magemods_audio_api", void AudioApi_StartSequence(u8 seqPlayerIndex, s32 seqId, u16 seqArgs, u16 fadeInDuration));

extern s32 ShrinkWindow_Letterbox_GetSize(void);

// Update the X/Y positions of these cursors before calling the function. They are declared outside the function, so it should be good.
RECOMP_HOOK("KaleidoScope_UpdateCursorSize") void insert_music_buttons(PlayState* play)
{
    sQuestStatusCursorsX[QUEST_MUSIC_MENU] = -71.0f;
    sQuestStatusCursorsY[QUEST_MUSIC_MENU] = -51.0f;
}

RECOMP_DECLARE_EVENT(music_buttons_interact(u16 cursor, PlayState* play));
RECOMP_DECLARE_EVENT(music_menu_open(PlayState* play));
RECOMP_DECLARE_EVENT(music_menu_close(PlayState* play));

extern bool should_skip_song_title_display[0x7F];

RecompuiResource music_menu_create_slot_settings()
{
    // Not implemented
    return false;
}

RecompuiResource music_menu_create_select_track()
{
    return false;
}

RecompuiResource music_menu_create_edit_title()
{
    return false;
}

RecompuiResource music_menu_create_track_settings()
{
    return false;
}

unsigned long music_menu_reroll_slot()
{
    return false;
}

void music_menu_init_icons()
{
    settingsIcon = recompui_create_texture_rgba32(settingsIconData, BUTTON_WIDTH, BUTTON_HEIGHT);
    openDropdownIcon = recompui_create_texture_rgba32(openDropdownIconData, BUTTON_WIDTH, BUTTON_HEIGHT);
    pencilIcon = recompui_create_texture_rgba32(pencilIconData, BUTTON_WIDTH_SMALL, BUTTON_HEIGHT_SMALL);
    dieIcon = recompui_create_texture_rgba32(dieIconData, BUTTON_WIDTH, BUTTON_HEIGHT);
    speakerIcon = recompui_create_texture_rgba32(speakerIconData, BUTTON_WIDTH, BUTTON_HEIGHT);
}

MusicMenu_Volume music_menu_create_volume(RecompuiResource parent, cTrack* track)
{
    MusicMenu_Volume volume;
    volume._base.parent = parent;
    volume._base.container = recompui_create_element(musicMenuContext, parent);

    // recompui_set_width(volume._base.container, 60.0f, UNIT_PERCENT);

    recompui_set_align_items(volume._base.container, ALIGN_ITEMS_CENTER);
    recompui_set_display(volume._base.container, DISPLAY_FLEX);
    recompui_set_flex_grow(volume._base.container, 1.0f);

    recompui_set_margin(volume._base.container, 10.0f, UNIT_DP);
    
    volume.iconContainer = recompui_create_element(musicMenuContext, volume._base.container);
    recompui_set_width(volume.iconContainer, BUTTON_WIDTH, UNIT_DP);
    recompui_set_height(volume.iconContainer, BUTTON_HEIGHT, UNIT_DP);
    recompui_set_margin_right(volume.iconContainer, 5.0f, UNIT_DP);

    volume.icon = recompui_create_imageview(musicMenuContext, volume.iconContainer, speakerIcon);

    volume.slider = recompui_create_slider(musicMenuContext, volume._base.container, SLIDERTYPE_PERCENT, 0.0f, 200.0f, 1.0f, 100.0f);
    if (track->slotIdx == NA_BGM_TERMINA_FIELD)
    {
        recomp_printf("Created slider at %p.\n", volume.slider);
    }
    recompui_set_width(volume.slider, 100.0f, UNIT_PERCENT);

    return volume;
}

MusicMenu_Icon_Button music_menu_create_icon_button(RecompuiTextureHandle icon, RecompuiResource parent, unsigned long width, unsigned long height, unsigned long (*callback)())
{
    MusicMenu_Icon_Button button;
    button._base.parent = parent;
    button._base.container = recompui_create_element(musicMenuContext, parent);

    
    recompui_set_width(button._base.container, width + 4, UNIT_DP);
    recompui_set_height(button._base.container, 50.0f, UNIT_PERCENT);
    
    button.icon = recompui_create_imageview(musicMenuContext, button._base.container, icon);
    recompui_set_padding(button.icon, 2.0f, UNIT_DP);

    button.callback = callback;

    return button;
}

MusicMenu_Slot_Column music_menu_create_slot_column(cTrack* slot, RecompuiResource parent)
{
    MusicMenu_Slot_Column column;
    column.cell.parent = parent;

    column.cell.container = recompui_create_element(musicMenuContext, parent);
    recompui_set_display(column.cell.container, DISPLAY_TABLE_CELL);

    recompui_set_min_width(column.cell.container, 15.0f, UNIT_PERCENT);
    recompui_set_max_width(column.cell.container, 15.0f, UNIT_PERCENT);
    recompui_set_min_height(column.cell.container, 15.0f, UNIT_PERCENT);
    recompui_set_max_height(column.cell.container, 15.0f, UNIT_PERCENT);
    recompui_set_border_width(column.cell.container, 2.0f, UNIT_DP);

    column._base.parent = column.cell.parent;
    column._base.container = recompui_create_element(musicMenuContext, column.cell.container);
    
    recompui_set_flex_direction(column._base.container, FLEX_DIRECTION_ROW);
    recompui_set_display(column._base.container, DISPLAY_FLEX);
    recompui_set_align_items(column._base.container, ALIGN_ITEMS_CENTER);

    recompui_set_height(column._base.container, 100.0f, UNIT_PERCENT);
    recompui_set_width(column._base.container, 100.0f, UNIT_PERCENT);

    column.slotIdx = slot->slotIdx;
    column.name = slot->slotName;
    column.label = recompui_create_label(musicMenuContext, column._base.container, column.name, LABELSTYLE_NORMAL);
    recompui_set_flex_grow(column.label, 1.0f); 
    recompui_set_text_align(column.label, TEXT_ALIGN_CENTER);
    recompui_set_line_height(column.label, 90.0f, UNIT_PERCENT);
    recompui_set_font_size(column.label, 100.0f, UNIT_PERCENT);
    recompui_set_overflow(column.label, OVERFLOW_HIDDEN);
    recompui_set_width(column.label, 100.0f, UNIT_PERCENT);

    column.buttonsContainer = recompui_create_element(musicMenuContext, column._base.container);
    recompui_set_height(column.buttonsContainer, 100.0f, UNIT_PERCENT);
    recompui_set_width(column.buttonsContainer, 32.0f, UNIT_DP);
    recompui_set_display(column.buttonsContainer, DISPLAY_BLOCK);
    recompui_set_margin(column.buttonsContainer, 2.0f, UNIT_DP);
    recompui_set_margin_right(column.buttonsContainer, 11.0f, UNIT_DP);

    column.settings = music_menu_create_icon_button(settingsIcon, column.buttonsContainer, BUTTON_WIDTH, BUTTON_HEIGHT, music_menu_create_slot_settings);
    recompui_set_border_left_width(column.settings._base.container, 2.0f, UNIT_DP);
    recompui_set_border_bottom_width(column.settings._base.container, 1.0f, UNIT_DP);
    
    column.selectTrack = music_menu_create_icon_button(openDropdownIcon, column.buttonsContainer, BUTTON_WIDTH, BUTTON_HEIGHT, music_menu_create_select_track);
    recompui_set_border_left_width(column.selectTrack._base.container, 2.0f, UNIT_DP);
    recompui_set_border_top_width(column.selectTrack._base.container, 1.0f, UNIT_DP);

    return column;
}

MusicMenu_Track_Column music_menu_create_track_column(cTrack* slot, RecompuiResource parent)
{
    MusicMenu_Track_Column column;
    column.cell.parent = parent;

    column.cell.container = recompui_create_element(musicMenuContext, parent);
    recompui_set_display(column.cell.container, DISPLAY_TABLE_CELL);

    recompui_set_width_auto(column.cell.container);
    recompui_set_min_height(column.cell.container, 15.0f, UNIT_PERCENT);
    recompui_set_max_height(column.cell.container, 15.0f, UNIT_PERCENT);

    column._base.parent = column.cell.parent;
    column._base.container = recompui_create_element(musicMenuContext, column.cell.container);
    
    recompui_set_flex_direction(column._base.container, FLEX_DIRECTION_ROW);
    recompui_set_display(column._base.container, DISPLAY_FLEX);
    recompui_set_align_items(column._base.container, ALIGN_ITEMS_CENTER);

    recompui_set_height(column._base.container, 100.0f, UNIT_PERCENT);
    recompui_set_width_auto(column._base.container);
    recompui_set_border_width(column._base.container, 2.0f, UNIT_DP);
    recompui_set_padding_left(column._base.container, 20.0f, UNIT_DP);
    
    column.albumArt._base.container = recompui_create_element(musicMenuContext, column._base.container);
    recompui_set_min_width(column.albumArt._base.container, 64.0f, UNIT_DP);
    recompui_set_max_width(column.albumArt._base.container, 64.0f, UNIT_DP);
    recompui_set_height(column.albumArt._base.container, 64.0f, UNIT_DP);
    recompui_set_border_width(column.albumArt._base.container, 3.0f, UNIT_DP);

    column.name = slot->name;
    column.titleContainer = recompui_create_element(musicMenuContext, column._base.container);
    recompui_set_display(column.titleContainer, DISPLAY_FLEX);
    recompui_set_flex_direction(column.titleContainer, FLEX_DIRECTION_ROW);
    recompui_set_width(column.titleContainer, 40.0f, UNIT_PERCENT);
    recompui_set_overflow_x(column.titleContainer, OVERFLOW_HIDDEN);
    recompui_set_overflow_y(column.titleContainer, OVERFLOW_HIDDEN);
    recompui_set_height(column.titleContainer, 100.0f, UNIT_PERCENT);
    recompui_set_margin_left(column.titleContainer, 20.0f, UNIT_DP);
    recompui_set_align_items(column.titleContainer, ALIGN_ITEMS_CENTER);
    // recompui_set_padding_bottom(column.titleContainer, 20.0f, UNIT_DP);

    column.label = recompui_create_label(musicMenuContext, column.titleContainer, column.name, LABELSTYLE_NORMAL);
    // recompui_set_height(column.label, 100.0f, UNIT_PERCENT);
    recompui_set_min_width(column.label, 1920.0f, UNIT_DP);
    // recompui_set_overflow(column.label, OVERFLOW_HIDDEN);
    
    column.editTitle = music_menu_create_icon_button(pencilIcon, column.titleContainer, BUTTON_WIDTH_SMALL, BUTTON_HEIGHT_SMALL, music_menu_create_edit_title);
    recompui_set_margin_top_auto(column.editTitle._base.container);

    column.volume = music_menu_create_volume(column._base.container, slot);

    column.buttonsContainer = recompui_create_element(musicMenuContext, column._base.container);
    recompui_set_height(column.buttonsContainer, 100.0f, UNIT_PERCENT);
    recompui_set_margin_left_auto(column.buttonsContainer);

    column.trackSettings = music_menu_create_icon_button(settingsIcon, column.buttonsContainer, BUTTON_WIDTH, BUTTON_HEIGHT, music_menu_create_track_settings);
    recompui_set_border_left_width(column.trackSettings._base.container, 2.0f, UNIT_DP);
    recompui_set_border_bottom_width(column.trackSettings._base.container, 1.0f, UNIT_DP);

    column.rerollSlot = music_menu_create_icon_button(dieIcon, column.buttonsContainer, BUTTON_WIDTH, BUTTON_HEIGHT, music_menu_reroll_slot);
    recompui_set_border_left_width(column.rerollSlot._base.container, 2.0f, UNIT_DP);
    recompui_set_border_top_width(column.rerollSlot._base.container, 1.0f, UNIT_DP);

    return column;
}

MusicMenu_Track_Element music_menu_create_row(cTrack* slot, RecompuiResource parent)
{
    MusicMenu_Track_Element row; 
    row._base.parent = parent;
    row._base.container = recompui_create_element(musicMenuContext, parent);

    recompui_set_width(row._base.container, 100.0f, UNIT_PERCENT);

    recompui_set_display(row._base.container, DISPLAY_TABLE_ROW);
    // recompui_set_align_items(row._base.container, ALIGN_ITEMS_CENTER);

    recompui_set_min_height(row._base.container, 15.0f, UNIT_PERCENT);
    recompui_set_max_height(row._base.container, 15.0f, UNIT_PERCENT);

    row.track = slot;
    row.slotCol = music_menu_create_slot_column(slot, row._base.container);
    row.trackCol = music_menu_create_track_column(slot, row._base.container);

    return row;
}

RECOMP_CALLBACK(".", music_rando_randomization_complete) void create_music_menu(cTrack* randomized)
{
    const float body_padding = 64.0f;
    const float container_height = RECOMPUI_TOTAL_HEIGHT - (2 * body_padding);
    const float container_width = container_height * (16.0f / 9.0f);
    const float container_border_width = 1.1f;
    const float container_border_radius = 16.0f;

    musicMenuContext = recompui_create_context();
    recompui_open_context(musicMenuContext);

    music_menu_init_icons();

    recompui_set_context_captures_input(musicMenuContext, false);
    recompui_set_context_captures_mouse(musicMenuContext, true);

    musicMenu._base.parent = recompui_context_root(musicMenuContext);

    recompui_set_position(musicMenu._base.parent, POSITION_ABSOLUTE);
    recompui_set_display(musicMenu._base.parent, DISPLAY_FLEX);

    recompui_set_top(musicMenu._base.parent, 0, UNIT_PERCENT);
    recompui_set_left(musicMenu._base.parent, 0, UNIT_PERCENT);
    recompui_set_bottom(musicMenu._base.parent, 100, UNIT_PERCENT);
    recompui_set_right(musicMenu._base.parent, 100, UNIT_PERCENT);

    recompui_set_width(musicMenu._base.parent, 100, UNIT_PERCENT);
    recompui_set_min_width(musicMenu._base.parent, 100, UNIT_PERCENT);
    recompui_set_max_width(musicMenu._base.parent, 100, UNIT_PERCENT);
    recompui_set_height(musicMenu._base.parent, 100, UNIT_PERCENT);
    recompui_set_min_height(musicMenu._base.parent, 100, UNIT_PERCENT);
    recompui_set_max_height(musicMenu._base.parent, 100, UNIT_PERCENT);

    recompui_set_align_items(musicMenu._base.parent, ALIGN_ITEMS_CENTER);
    recompui_set_justify_content(musicMenu._base.parent, JUSTIFY_CONTENT_CENTER);

    musicMenu._base.container = recompui_create_element(musicMenuContext, musicMenu._base.parent);
    
    // Center the thing where the map viewport is

    recompui_set_display(musicMenu._base.container, DISPLAY_INLINE_BLOCK);
    recompui_set_overflow_y(musicMenu._base.container, OVERFLOW_SCROLL);
    recompui_set_margin_top(musicMenu._base.container, 53.0f, UNIT_DP);

    recompui_set_width(musicMenu._base.container, 0.90f * RECOMPUI_TOTAL_HEIGHT, UNIT_DP);
    // recompui_set_max_width(container, 70, UNIT_PERCENT);
    recompui_set_border_width(musicMenu._base.container, 2, UNIT_DP);
    recompui_set_border_color(musicMenu._base.container, &white);

    recompui_set_height(musicMenu._base.container, 0.533f * RECOMPUI_TOTAL_HEIGHT, UNIT_DP);

    recompui_set_background_color(musicMenu._base.container, &container_color);

    musicMenu.trackTable = recompui_create_element(musicMenuContext, musicMenu._base.container);
    recompui_set_display(musicMenu.trackTable, DISPLAY_TABLE);
    recompui_set_width(musicMenu.trackTable, 100.0f, UNIT_PERCENT);
    recompui_set_height(musicMenu.trackTable, 100.0f, UNIT_PERCENT);
    recompui_set_border_top_width(musicMenu.trackTable, 1.0f, UNIT_DP);

    musicMenu.tracks = recomp_alloc(sizeof(MusicMenu_Track_Element*) * (NA_BGM_MAX - 2));

    for (int i = NA_BGM_TERMINA_FIELD; i < NA_BGM_MAX; i++)
    {
        if (should_skip_song_title_display[i])
        {
            continue;
        }
        MusicMenu_Track_Element row = music_menu_create_row(&randomized[i], musicMenu.trackTable);

        musicMenu.tracks[i] = recomp_alloc(sizeof(MusicMenu_Track_Element));
        Lib_MemCpy(musicMenu.tracks[i], &row, sizeof(MusicMenu_Track_Element));
    }

    // recompui_set_border_color(musicMenu.tracks[NA_BGM_TERMINA_FIELD]->slotCol.cell.container, &aBlue);
    // recompui_set_border_width(musicMenu.tracks[NA_BGM_TERMINA_FIELD]->slotCol.cell.container, 10.0f, UNIT_DP);
    // recompui_set_border_color(musicMenu.tracks[NA_BGM_TERMINA_FIELD]->_base.container, &red);
    // recompui_set_border_width(musicMenu.tracks[NA_BGM_TERMINA_FIELD]->_base.container, 10.0f, UNIT_DP);

    recompui_close_context(musicMenuContext);
}

RECOMP_HOOK_RETURN("AudioSeq_UpdateActiveSequences") void update_volume()
{
    int seqId = gAudioCtx.seqPlayers[SEQ_PLAYER_BGM_MAIN].seqId;
    // logger.dev("fadeVolumeScale: %f\n", gAudioCtx.seqPlayers[SEQ_PLAYER_BGM_MAIN].fadeVolumeScale);
    if (is_music_menu_open)
    {
        recompui_open_context(musicMenuContext);
        volumeVals[seqId] = recompui_get_input_value_float(musicMenu.tracks[seqId]->trackCol.volume.slider) * 127.0f / 100.0f;
        recompui_close_context(musicMenuContext);
    }
    if (has_music_menu_been_opened)
    {
        f32 cur_volume = gActiveSeqs[SEQ_PLAYER_BGM_MAIN].volCur;
        cur_volume *= volumeVals[gActiveSeqs[SEQ_PLAYER_BGM_MAIN].seqId] / 127.0f;
        AUDIOCMD_SEQPLAYER_FADE_VOLUME_SCALE(SEQ_PLAYER_BGM_MAIN, cur_volume);
    }
}

RECOMP_HOOK("KaleidoScope_UpdateQuestCursor") void check_music_menu_close(PlayState* play)
{
    if (is_music_menu_open)
    {
        if (CHECK_BTN_ANY(CONTROLLER1(&play->state)->press.button, BTN_B))
        {
            Audio_PlaySfx(NA_SE_SY_DECIDE);
            recompui_hide_context(musicMenuContext);
            music_menu_close(play);
            is_music_menu_open = false;
        }
    }
}

RECOMP_CALLBACK(".", music_buttons_interact) void on_music_buttons_interact(u16 cursor, PlayState* play)
{
    s32 seqIdPrev = seqId;
    u16 seqArgsPrev = seqArgs;

    seqId = AudioApi_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);
    seqArgs = AudioApi_GetActiveSeqArgs(SEQ_PLAYER_BGM_MAIN);

    if (seqId == 65535)
    {
        seqId = seqIdPrev;
        seqArgs = seqArgsPrev;
    }

    else if (cursor == QUEST_MUSIC_MENU)
    {
        if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_A))
        {
            if (!is_music_menu_open)
            {
                Audio_PlaySfx(NA_SE_SY_DECIDE);
                play->pauseCtx.state = PAUSE_STATE_SAVEPROMPT;
                play->pauseCtx.savePromptState = PAUSE_MUSICMENU_STATE_APPEARING;
            }
        }
    }
}

extern f32 D_8082B90C;
extern s16 sPauseCursorLeftX;
extern s16 sPauseCursorRightX;

extern f32 sPauseCursorLeftMoveOffsetX = 20.0f;
extern f32 sPauseCursorRightMoveOffsetX = -20.0f;
extern f32 sPauseMenuVerticalOffset;

RECOMP_HOOK("KaleidoScope_Update") void Pre_KaleidoScope_OpenMusicMenu(PlayState* play)
{
    // log_debug("questPageRoll: %f\n", play->pauseCtx.questPageRoll);
    // log_debug("roll: %f\n", play->pauseCtx.roll);

    Input* input = CONTROLLER1(&play->state);
    PauseContext* pauseCtx = &play->pauseCtx;
    
    switch (pauseCtx->state)
    {
        case PAUSE_STATE_SAVEPROMPT:
            switch (pauseCtx->savePromptState)
            {
                case PAUSE_MUSICMENU_STATE_APPEARING:
                        pauseCtx->questPageRoll -= 78.5f;
                        sPauseCursorLeftX -= TRUNCF_BINANG(sPauseCursorLeftMoveOffsetX / 4);
                        sPauseCursorRightX -= TRUNCF_BINANG(sPauseCursorRightMoveOffsetX / 4);
                        if (pauseCtx->questPageRoll <= -628.0f) {
                            pauseCtx->questPageRoll = -628.0f;
                            pauseCtx->state = PAUSE_STATE_SAVEPROMPT;
                            pauseCtx->savePromptState = PAUSE_MUSICMENU_STATE_IDLE;
                        }
                    break;
                case PAUSE_MUSICMENU_STATE_IDLE:
                    if (!is_music_menu_open)
                    {
                        recompui_show_context(musicMenuContext);
                        is_music_menu_open = true;
                        has_music_menu_been_opened = true;
                        music_menu_open(play);
                    }

                    if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
                        Interface_SetAButtonDoAction(play, DO_ACTION_NONE);
                        pauseCtx->savePromptState = PAUSE_MUSICMENU_STATE_CLOSING;
                        D_8082B90C = pauseCtx->questPageRoll;
                    }
                    else if (CHECK_BTN_ALL(input->press.button, BTN_START))
                    {
                        Interface_SetAButtonDoAction(play, DO_ACTION_NONE);
                        pauseCtx->state = PAUSE_STATE_UNPAUSE_SETUP;
                        sPauseMenuVerticalOffset = -6240.0f;
                        Audio_PlaySfx_PauseMenuOpenOrClose(SFX_PAUSE_MENU_CLOSE);
                        pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
                        recompui_hide_context(musicMenuContext);
                        music_menu_close(play);
                        is_music_menu_open = false;
                    }
                    break;
                case PAUSE_MUSICMENU_STATE_CLOSING:
                        pauseCtx->questPageRoll += 78.5f;
                        sPauseCursorLeftX += TRUNCF_BINANG(sPauseCursorLeftMoveOffsetX / 4);
                        sPauseCursorRightX += TRUNCF_BINANG(sPauseCursorRightMoveOffsetX / 4);
                        if (pauseCtx->questPageRoll >= 0.0f) {
                            pauseCtx->questPageRoll = 0.0f;
                            pauseCtx->state = PAUSE_STATE_MAIN;
                        }
                    break;
            }
            break;
    }
}

extern f32 sPauseMenuVerticalOffset;
extern TexturePtr sMapPageBgTextures[];
PlayState* thisPlay;
GraphicsContext* thisGfxCtx;

extern Gfx* KaleidoScope_DrawPageSections(Gfx* gfx, Vtx* vertices, TexturePtr* textures);

RECOMP_HOOK("KaleidoScope_DrawPages") void pre_replace_quest_texture(PlayState* play, GraphicsContext* gfxCtx)
{
    thisPlay = play;
    thisGfxCtx = gfxCtx;
}

// Unfortunately I have to RECOMP_PATCH this one, as sCursorPointLinks is defined within the function.
// Actually that like. actually omegasucks

RECOMP_PATCH void KaleidoScope_UpdateQuestCursor(PlayState* play) 
{
    static s16 sQuestSongPlaybackDelayTimer = 0;

    PauseContext* pauseCtx = &play->pauseCtx;
    MessageContext* msgCtx = &play->msgCtx;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s32 pad;
    s16 nextCursorPoint;
    s16 oldCursorPoint;
    s16 i;
    u16 cursor;
    u16 cursorItem;

    pauseCtx->nameColorSet = PAUSE_NAME_COLOR_SET_WHITE;
    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;

    // != PAUSE_MAIN_STATE_IDLE
    if ((pauseCtx->state == PAUSE_STATE_MAIN) &&
        (!pauseCtx->mainState || (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) ||
         (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG)) &&
        (pauseCtx->pageIndex == PAUSE_QUEST) && !pauseCtx->itemDescriptionOn) {
        if (pauseCtx->cursorSpecialPos == 0) {
            oldCursorPoint = pauseCtx->cursorPoint[PAUSE_QUEST];

            if (pauseCtx->stickAdjX < -30) {
                // Move cursor left
                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }
                pauseCtx->cursorShrinkRate = 4.0f;

                nextCursorPoint = sCursorPointLinksButWithMusicMenu[oldCursorPoint].left;
                if (nextCursorPoint == CURSOR_TO_LEFT) {
                    KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
                    pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
                    if (interfaceCtx->aButtonDoActionDelayed == DO_ACTION_DECIDE) {
                        Interface_SetAButtonDoAction(play, DO_ACTION_INFO);
                    }
                    return;
                } else {
                    while (nextCursorPoint > CURSOR_NONE) {
                        if (KaleidoScope_UpdateQuestStatusPoint(pauseCtx, nextCursorPoint)) {
                            break;
                        }
                        nextCursorPoint = sCursorPointLinksButWithMusicMenu[nextCursorPoint].left;
                    }
                }
            } else if (pauseCtx->stickAdjX > 30) {
                // Move cursor right
                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }
                pauseCtx->cursorShrinkRate = 4.0f;
                nextCursorPoint = sCursorPointLinksButWithMusicMenu[oldCursorPoint].right;

                if (nextCursorPoint == CURSOR_TO_RIGHT) {
                    KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);
                    pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
                    return;
                }

                while (nextCursorPoint > CURSOR_NONE) {
                    if (KaleidoScope_UpdateQuestStatusPoint(pauseCtx, nextCursorPoint)) {
                        break;
                    }
                    nextCursorPoint = sCursorPointLinksButWithMusicMenu[nextCursorPoint].right;
                }
            }

            if (pauseCtx->stickAdjY < -30) {
                // Move cursor down
                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }
                nextCursorPoint = sCursorPointLinksButWithMusicMenu[oldCursorPoint].down;

                while (nextCursorPoint > CURSOR_NONE) {
                    pauseCtx->cursorShrinkRate = 4.0f;
                    if (KaleidoScope_UpdateQuestStatusPoint(pauseCtx, nextCursorPoint)) {
                        break;
                    }
                    nextCursorPoint = sCursorPointLinksButWithMusicMenu[nextCursorPoint].down;
                }
            } else if (pauseCtx->stickAdjY > 30) {
                // Move cursor up
                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }
                nextCursorPoint = sCursorPointLinksButWithMusicMenu[oldCursorPoint].up;
                while (nextCursorPoint > CURSOR_NONE) {
                    pauseCtx->cursorShrinkRate = 4.0f;
                    if (KaleidoScope_UpdateQuestStatusPoint(pauseCtx, nextCursorPoint)) {
                        break;
                    }
                    nextCursorPoint = sCursorPointLinksButWithMusicMenu[nextCursorPoint].up;
                }
            }

            // if the cursor point changed
            if (oldCursorPoint != pauseCtx->cursorPoint[PAUSE_QUEST]) {
                pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            }

            // Update cursor item and slot
            if (pauseCtx->cursorPoint[PAUSE_QUEST] != QUEST_HEART_PIECE) {
                if (pauseCtx->cursorPoint[PAUSE_QUEST] <= QUEST_REMAINS_TWINMOLD) {
                    // Boss Remains
                    if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                        cursorItem = ITEM_REMAINS_ODOLWA + pauseCtx->cursorPoint[PAUSE_QUEST];
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else if (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_BOMBERS_NOTEBOOK) {
                    // Bombers Notebook
                    if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                        cursorItem = ITEM_BOMBERS_NOTEBOOK;
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else if (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_SHIELD) {
                    // Shield
                    if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) != EQUIP_VALUE_SHIELD_NONE) {
                        cursorItem = (ITEM_SHIELD_HERO - EQUIP_TYPE_SHIELD) + GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD);
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else if (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_SWORD) {
                    // Sword
                    if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) != EQUIP_VALUE_SWORD_NONE) {
                        cursorItem =
                            (ITEM_SWORD_KOKIRI - EQUIP_VALUE_SWORD_KOKIRI) + GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD);
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else if (pauseCtx->cursorPoint[PAUSE_QUEST] <= QUEST_SONG_SUN) {
                    // Songs
                    if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                        cursorItem = ITEM_WALLET_GIANT + pauseCtx->cursorPoint[PAUSE_QUEST];
                    } else if ((pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_SONG_LULLABY) &&
                               CHECK_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO)) {
                        cursorItem = ITEM_SONG_LULLABY_INTRO;
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else if (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_QUIVER) {
                    // Quiver Upgrade
                    if (GET_CUR_UPG_VALUE(UPG_QUIVER) != 0) {
                        cursorItem = (ITEM_QUIVER_30 - 1) + GET_CUR_UPG_VALUE(UPG_QUIVER);
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else if (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_BOMB_BAG) {
                    // Bomb Bag Upgrade
                    if (GET_CUR_UPG_VALUE(UPG_BOMB_BAG) != 0) {
                        cursorItem = (ITEM_BOMB_BAG_20 - 1) + GET_CUR_UPG_VALUE(UPG_BOMB_BAG);
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else {
                    cursorItem = PAUSE_ITEM_NONE;
                }
            } else {
                // Heart Piece Count
                if ((GET_SAVE_INVENTORY_QUEST_ITEMS >> QUEST_HEART_PIECE_COUNT) != 0) {
                    cursorItem = ITEM_HEART_CONTAINER;
                } else {
                    cursorItem = PAUSE_ITEM_NONE;
                }
            }

            cursor = pauseCtx->cursorPoint[PAUSE_QUEST];
            pauseCtx->cursorItem[pauseCtx->pageIndex] = cursorItem;
            pauseCtx->cursorSlot[pauseCtx->pageIndex] = cursor;

            if ((pauseCtx->debugEditor == DEBUG_EDITOR_NONE) && (pauseCtx->state == PAUSE_STATE_MAIN) &&
                (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) && (pauseCtx->cursorSpecialPos == 0)) {
                if ((cursor >= QUEST_SONG_SONATA) && (cursor <= QUEST_SONG_SUN) && !(cursor == QUEST_MUSIC_MENU)) {
                    // Handle part of the ocarina songs playback
                    if ((CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST]) ||
                         ((cursor == QUEST_SONG_LULLABY) && !CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST]) &&
                          CHECK_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO))) &&
                        (msgCtx->msgLength == 0)) {
                        // The cursor is on a learned song
                        // Set some things up for song playback

                        if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                            cursor = pauseCtx->cursorSlot[PAUSE_QUEST];
                        } else {
                            cursor = QUEST_BOMB_BAG;
                        }

                        pauseCtx->ocarinaSongIndex = gOcarinaSongItemMap[cursor - QUEST_SONG_SONATA];
                        sQuestSongPlaybackDelayTimer = 10;

                        for (i = 0; i < 8; i++) {
                            sQuestSongPlayedOcarinaButtons[i] = OCARINA_BTN_INVALID;
                            sQuestSongPlayedOcarinaButtonsAlpha[i] = 0;
                        }

                        sQuestSongPlayedOcarinaButtonsNum = 0;

                        // Setup the song to receive user input, immediately cancelled below
                        AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_DEFAULT);
                        AudioOcarina_StartDefault((1 << pauseCtx->ocarinaSongIndex) | 0x80000000);

                        // Clear the playback staff
                        pauseCtx->ocarinaStaff = AudioOcarina_GetPlaybackStaff();
                        pauseCtx->ocarinaStaff->pos = 0;
                        pauseCtx->ocarinaStaff->state = 0xFF;

                        pauseCtx->ocarinaButtonsY[OCARINA_BTN_A] = -62;
                        pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_DOWN] = -56;
                        pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_RIGHT] = -49;
                        pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_LEFT] = -46;
                        pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_UP] = -41;

                        // pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG;

                        if (interfaceCtx->aButtonDoActionDelayed != DO_ACTION_DECIDE) {
                            Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);
                        }

                        // Stop receiving input to play a song as mentioned above
                        AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);

                        if (gSaveContext.buttonStatus[EQUIP_SLOT_A] == BTN_DISABLED) {
                            gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_ENABLED;
                            gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
                            Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
                        }
                    } else {
                        if (interfaceCtx->aButtonDoActionDelayed != DO_ACTION_DECIDE) {
                            Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);
                        }
                        if (gSaveContext.buttonStatus[EQUIP_SLOT_A] != BTN_DISABLED) {
                            gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_DISABLED;
                            gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
                            Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
                        }
                    }
                } 
                else if (cursor == QUEST_MUSIC_MENU)
                {
                    music_buttons_interact(cursor, play);
                }
                else {
                    if ((cursor == QUEST_BOMBERS_NOTEBOOK) && (pauseCtx->cursorItem[PAUSE_QUEST] != PAUSE_ITEM_NONE)) {
                        if (interfaceCtx->aButtonDoActionDelayed != DO_ACTION_DECIDE) {
                            Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);
                        }
                        pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_BLUE;
                    } else if (interfaceCtx->aButtonDoActionDelayed == DO_ACTION_DECIDE) {
                        Interface_SetAButtonDoAction(play, DO_ACTION_INFO);
                    }

                    if ((pauseCtx->cursorItem[PAUSE_QUEST] != PAUSE_ITEM_NONE) && (msgCtx->msgLength == 0)) {
                        if (gSaveContext.buttonStatus[EQUIP_SLOT_A] == BTN_DISABLED) {
                            gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_ENABLED;
                            gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
                            Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
                        }

                        if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_A) && (msgCtx->msgLength == 0)) {
                            if (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_BOMBERS_NOTEBOOK) {
                                play->pauseCtx.bombersNotebookOpen = true;
                                pauseCtx->mainState = PAUSE_MAIN_STATE_BOMBERS_NOTEBOOK_OPEN;
                                Audio_PlaySfx(NA_SE_SY_DECIDE);
                            } else {
                                pauseCtx->itemDescriptionOn = true;
                                if (pauseCtx->cursorYIndex[PAUSE_QUEST] < 2) {
                                    if (pauseCtx->cursorItem[PAUSE_QUEST] < ITEM_REMAINS_ODOLWA) {
                                        func_801514B0(play, 0x1737 + pauseCtx->cursorItem[PAUSE_QUEST], 1);
                                    } else {
                                        func_801514B0(play, 0x173B + pauseCtx->cursorItem[PAUSE_QUEST], 3);
                                    }
                                } else {
                                    if (pauseCtx->cursorItem[PAUSE_QUEST] < ITEM_REMAINS_ODOLWA) {
                                        func_801514B0(play, 0x1737 + pauseCtx->cursorItem[PAUSE_QUEST], 1);
                                    } else {
                                        func_801514B0(play, 0x173B + pauseCtx->cursorItem[PAUSE_QUEST], 1);
                                    }
                                }
                            }
                        }
                    } else if (gSaveContext.buttonStatus[EQUIP_SLOT_A] != BTN_DISABLED) {
                        gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_DISABLED;
                        gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
                        Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
                    }
                }
            } else if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                // Abort reading ocarina song input if the stick is moved
                if ((pauseCtx->stickAdjX != 0) || (pauseCtx->stickAdjY != 0)) {
                    pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }
            } else if ((pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG) &&
                       CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_A) && (msgCtx->msgLength == 0) &&
                       (cursor >= QUEST_SONG_SONATA) && (cursor <= QUEST_SONG_SUN)) {
                pauseCtx->mainState = PAUSE_MAIN_STATE_SONG_PLAYBACK_INIT;
                sQuestSongPlaybackDelayTimer = 10;
            }

            if (pauseCtx->cursorSpecialPos == 0) {
                if ((pauseCtx->cursorSlot[PAUSE_QUEST] >= 6) && (pauseCtx->cursorSlot[PAUSE_QUEST] < 0x12) &&
                    ((pauseCtx->mainState <= PAUSE_MAIN_STATE_SONG_PLAYBACK) ||
                     (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) ||
                     (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG)) &&
                    (pauseCtx->cursorItem[pauseCtx->pageIndex] != PAUSE_ITEM_NONE)) {
                    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_BLUE;
                    if ((pauseCtx->mainState >= PAUSE_MAIN_STATE_SONG_PLAYBACK) &&
                        (pauseCtx->mainState <= PAUSE_MAIN_STATE_SONG_PROMPT_DONE)) {
                        pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;
                    }
                }
            }
        } else if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT) {
            if (pauseCtx->stickAdjX > 30) {
                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }

                KaleidoScope_MoveCursorFromSpecialPos(play);

                pauseCtx->cursorPoint[PAUSE_QUEST] = QUEST_BOMBERS_NOTEBOOK;
                if (CHECK_QUEST_ITEM(QUEST_BOMBERS_NOTEBOOK)) {
                    cursorItem = ITEM_BOMBERS_NOTEBOOK;
                } else {
                    cursorItem = PAUSE_ITEM_NONE;
                }

                cursor = pauseCtx->cursorPoint[PAUSE_QUEST];
                pauseCtx->cursorItem[pauseCtx->pageIndex] = cursorItem;
                pauseCtx->cursorSlot[pauseCtx->pageIndex] = cursor;
            }
        } else if (pauseCtx->stickAdjX < -30) {
            if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
            }

            KaleidoScope_MoveCursorFromSpecialPos(play);

            pauseCtx->cursorPoint[PAUSE_QUEST] = QUEST_REMAINS_GOHT;
            if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                cursorItem = (ITEM_REMAINS_GOHT - 1) + pauseCtx->cursorPoint[PAUSE_QUEST];
                if (pauseCtx->cursorPoint[PAUSE_QUEST] != QUEST_REMAINS_ODOLWA) {
                    // This condition is always true as `cursorPoint` is set three lines above
                    cursorItem = ITEM_MASK_GIANT;
                }
            } else {
                cursorItem = PAUSE_ITEM_NONE;
            }

            cursor = pauseCtx->cursorPoint[PAUSE_QUEST];
            pauseCtx->cursorItem[pauseCtx->pageIndex] = cursorItem;
            pauseCtx->cursorSlot[pauseCtx->pageIndex] = cursor;
        }
    } else if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PLAYBACK_INIT) {
        // After a short delay, start the playback of the selected song

        pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_BLUE;
        sQuestSongPlaybackDelayTimer--;
        if (sQuestSongPlaybackDelayTimer == 0) {
            for (i = 0; i < 8; i++) {
                sQuestSongPlayedOcarinaButtons[i] = OCARINA_BTN_INVALID;
                sQuestSongPlayedOcarinaButtonsAlpha[i] = 0;
            }
            sQuestSongPlayedOcarinaButtonsNum = 0;

            pauseCtx->ocarinaButtonsY[OCARINA_BTN_A] = -62;
            pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_DOWN] = -56;
            pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_RIGHT] = -49;
            pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_LEFT] = -46;
            pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_UP] = -41;

            if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                cursor = pauseCtx->cursorSlot[PAUSE_QUEST];
            } else {
                cursor = QUEST_BOMB_BAG;
            }

            AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_DEFAULT);
            AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_DEFAULT);
            pauseCtx->ocarinaSongIndex = gOcarinaSongItemMap[cursor - QUEST_SONG_SONATA];
            AudioOcarina_SetPlaybackSong(pauseCtx->ocarinaSongIndex + 1, 1);
            pauseCtx->mainState = PAUSE_MAIN_STATE_SONG_PLAYBACK;
            pauseCtx->ocarinaStaff = AudioOcarina_GetPlaybackStaff();
            pauseCtx->ocarinaStaff->pos = 0;
        }
    }
}

PlayState* sPlayState;
GraphicsContext* sGfxCtx;

RECOMP_HOOK("KaleidoScope_SetVertices") void music_buttons_init(PlayState* play, GraphicsContext* gfxCtx)
{
    sQuestVtxRectLeft[QUEST_MUSIC_MENU] = -100 / 0.78f;
    sQuestVtxRectTop[QUEST_MUSIC_MENU] = -45 / 0.78f;
    sQuestVtxWidths[QUEST_MUSIC_MENU] = 32;
    sQuestVtxHeights[QUEST_MUSIC_MENU] = 32;
}