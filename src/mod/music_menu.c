#include "music_menu.h"

extern f32 sQuestStatusCursorsX[];
extern f32 sQuestStatusCursorsY[];

extern s16 sQuestVtxRectLeft[];
extern s16 sQuestVtxRectTop[];

extern s16 sQuestVtxWidths[];
extern s16 sQuestVtxHeights[];

extern u8 sAudioPauseState;

RECOMP_IMPORT("magemods_audio_api", s32 AudioApi_GetActiveSeqId(u8 seqPlayerIndex));
RECOMP_IMPORT("magemods_audio_api", u16 AudioApi_GetActiveSeqArgs(u8 seqPlayerIndex));
RECOMP_IMPORT("magemods_audio_api", void AudioApi_StartSequence(u8 seqPlayerIndex, s32 seqId, u16 seqArgs, u16 fadeInDuration));

// Update the X/Y positions of these cursors before calling the function. They are declared outside the function, so it should be good.
RECOMP_HOOK("KaleidoScope_UpdateCursorSize") void insert_music_buttons(PlayState* play)
{
    sQuestStatusCursorsX[QUEST_TRACKS] = -58.0f;
    sQuestStatusCursorsX[QUEST_REWIND] = -35.0f;
    sQuestStatusCursorsX[QUEST_FORWARD] = -13.0f;

    sQuestStatusCursorsY[QUEST_TRACKS] = -45.0f;
    sQuestStatusCursorsY[QUEST_REWIND] = -45.0f;
    sQuestStatusCursorsY[QUEST_FORWARD] = -45.0f;
}

RECOMP_DECLARE_EVENT(music_buttons_interact(u16 cursor, PlayState* play));

RECOMP_CALLBACK(".", music_buttons_interact) void on_music_buttons_interact(u16 cursor, PlayState* play)
{
    if (cursor == QUEST_REWIND)
    {
        if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_A))
        {
            s32 seqId = AudioApi_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);
            u16 seqArgs = AudioApi_GetActiveSeqArgs(SEQ_PLAYER_BGM_MAIN);

            AudioSeq_StopSequence(SEQ_PLAYER_BGM_MAIN, 0);
            AudioApi_StartSequence(SEQ_PLAYER_BGM_MAIN, seqId, seqArgs, 0);
        }
    }
    else if (cursor == QUEST_FORWARD)
    {
        if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_A))
        {
            s32 seqId = AudioApi_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);
            u16 seqArgs = AudioApi_GetActiveSeqArgs(SEQ_PLAYER_BGM_MAIN);

            do
            {
                seqId++;
                seqId %= gAudioCtx.sequenceTable->header.numEntries;
            } while(gAudioCtx.sequenceTable->entries[seqId].romAddr == NULL);

            recomp_printf("Playing sequence with ID %i\n", seqId);

            AudioSeq_StopSequence(SEQ_PLAYER_BGM_MAIN, 0);
            AudioApi_StartSequence(SEQ_PLAYER_BGM_MAIN, seqId, seqArgs, 0);
        }
    }
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
                if ((cursor >= QUEST_SONG_SONATA) && (cursor <= QUEST_SONG_SUN) && !(cursor == QUEST_TRACKS || cursor == QUEST_REWIND || cursor == QUEST_FORWARD)) {
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

                        pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG;

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
                else if (cursor == QUEST_TRACKS || cursor == QUEST_REWIND || cursor == QUEST_FORWARD)
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
    sQuestVtxRectLeft[QUEST_TRACKS] = -80;
    sQuestVtxRectTop[QUEST_TRACKS] = -44;
    sQuestVtxWidths[QUEST_TRACKS] = 24;
    sQuestVtxHeights[QUEST_TRACKS] = 24;

    sQuestVtxRectLeft[QUEST_REWIND] = -56;
    sQuestVtxRectTop[QUEST_REWIND] = -44;
    sQuestVtxWidths[QUEST_REWIND] = 16;
    sQuestVtxHeights[QUEST_TRACKS] = 16;

    sQuestVtxRectLeft[QUEST_FORWARD] = -14;
    sQuestVtxRectTop[QUEST_FORWARD] = -44;
    sQuestVtxWidths[QUEST_TRACKS] = 16;
    sQuestVtxHeights[QUEST_TRACKS] = 16;
}