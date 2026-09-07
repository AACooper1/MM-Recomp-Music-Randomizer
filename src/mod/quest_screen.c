#include "quest_screen.h"

extern f32 sPauseMenuVerticalOffset;
extern TexturePtr sMapPageBgTextures[];
PlayState* gPlay;
GraphicsContext* gGfxCtx;
PauseContext* gPauseCtx;

extern Gfx* KaleidoScope_DrawPageSections(Gfx* gfx, Vtx* vertices, TexturePtr* textures);

RECOMP_HOOK("KaleidoScope_DrawPages") void before_KaleidoScope_DrawPages(PlayState* play, GraphicsContext* gfxCtx)
{
    gPlay = play;
    gGfxCtx = gfxCtx;
}

static f32 sStaffCoverRoll = 0.0f;

RECOMP_HOOK_RETURN("KaleidoScope_DrawPages") void cover_staff_texture()
{
    PlayState* play = gPlay;
    GraphicsContext* gfxCtx = gGfxCtx;

    PauseContext* pauseCtx = &play->pauseCtx;

    if (pauseCtx->questPageRoll <= -314.0f)
    {
        return;
    }
    
    u64* coverTexture;
    if (sStaffCoverRoll <= -628.0f)
    {
        return;
    }
    else if (sStaffCoverRoll <= -314.0f)
    {
        coverTexture = gMusicStaffTex;
    }
    else
    {
        coverTexture = gQuestPageStaffCoverTex;
    }

    OPEN_DISPS(gfxCtx);
    if (pauseCtx->pageIndex == PAUSE_QUEST)
    {
        static Vtx sCoverVtx[4];

        s16 width  = 128;
        s16 height = 32;
        s16 left   = -width / 2;
        s16 top    = height / 2;

        // top-left
        sCoverVtx[0].v.ob[0] = left;
        sCoverVtx[0].v.ob[1] = top;
        sCoverVtx[0].v.ob[2] = 0;
        sCoverVtx[0].v.flag  = 0;
        sCoverVtx[0].v.tc[0] = 0;
        sCoverVtx[0].v.tc[1] = 0;
        sCoverVtx[0].v.cn[0] = sCoverVtx[0].v.cn[1] = sCoverVtx[0].v.cn[2] = 255;
        sCoverVtx[0].v.cn[3] = pauseCtx->alpha;

        // top-right
        sCoverVtx[1].v.ob[0] = left + width;
        sCoverVtx[1].v.ob[1] = top;
        sCoverVtx[1].v.ob[2] = 0;
        sCoverVtx[1].v.flag  = 0;
        sCoverVtx[1].v.tc[0] = width << 5;
        sCoverVtx[1].v.tc[1] = 0;
        sCoverVtx[1].v.cn[0] = sCoverVtx[1].v.cn[1] = sCoverVtx[1].v.cn[2] = 255;
        sCoverVtx[1].v.cn[3] = pauseCtx->alpha;

        // bottom-left
        sCoverVtx[2].v.ob[0] = left;
        sCoverVtx[2].v.ob[1] = top - height;
        sCoverVtx[2].v.ob[2] = 0;
        sCoverVtx[2].v.flag  = 0;
        sCoverVtx[2].v.tc[0] = 0;
        sCoverVtx[2].v.tc[1] = height << 5;
        sCoverVtx[2].v.cn[0] = sCoverVtx[2].v.cn[1] = sCoverVtx[2].v.cn[2] = 255;
        sCoverVtx[2].v.cn[3] = pauseCtx->alpha;

        // bottom-right
        sCoverVtx[3].v.ob[0] = left + width;
        sCoverVtx[3].v.ob[1] = top - height;
        sCoverVtx[3].v.ob[2] = 0;
        sCoverVtx[3].v.flag  = 0;
        sCoverVtx[3].v.tc[0] = width << 5;
        sCoverVtx[3].v.tc[1] = height << 5;
        sCoverVtx[3].v.cn[0] = sCoverVtx[3].v.cn[1] = sCoverVtx[3].v.cn[2] = 255;
        sCoverVtx[3].v.cn[3] = pauseCtx->alpha;

        gDPPipeSync(POLY_OPA_DISP++);
        
        gDPSetCombineLERP(POLY_OPA_DISP++, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, PRIMITIVE, 0,
                                TEXEL0, 0, SHADE, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 180, 180, 120, 255);
        gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_BILERP);

        Matrix_Translate(-43.0f / 0.78f, -44.0f / 0.78f, 0.0, MTXMODE_APPLY);
        
        if (sStaffCoverRoll != -628.0f)
        {
            // Draw cover over background
            MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gfxCtx);
            gDPLoadTextureBlock(POLY_OPA_DISP++, gStaffCoverBlankTex, G_IM_FMT_IA, G_IM_SIZ_8b, 128, 32, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

            gSPVertex(POLY_OPA_DISP++, sCoverVtx, 4, 0);

            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        }



        Matrix_RotateXFApply((-sStaffCoverRoll / 100.0f));
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gfxCtx);
        
        gDPLoadTextureBlock(POLY_OPA_DISP++, coverTexture, G_IM_FMT_IA, G_IM_SIZ_8b, 128, 32, 0, G_TX_NOMIRROR | G_TX_WRAP,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

        gSPVertex(POLY_OPA_DISP++, sCoverVtx, 4, 0);

        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    }
    CLOSE_DISPS();
}

GraphicsContext* gGfxCtx;

#define ROT_90_DEGREES -314.0f

bool rotate_staff_cover_open(PlayState* play, GraphicsContext* gfxCtx)
{
    sStaffCoverRoll = MAX(sStaffCoverRoll - 78.5f, ROT_90_DEGREES * 2);
    return (sStaffCoverRoll == ROT_90_DEGREES * 2);
}

bool rotate_staff_cover_closed(PlayState* play, GraphicsContext* gfxCtx)
{
    sStaffCoverRoll = MIN(sStaffCoverRoll + 78.5f, 0.0f);
    return (sStaffCoverRoll == 0.0f);
}

extern u8 sQuestSongPlayedOcarinaButtons[];
extern s16 sQuestSongPlayedOcarinaButtonsAlpha[];
u8 gSwitchStatesTimer = 0;
bool is_staff_cover_shown = true;

RECOMP_HOOK("KaleidoScope_UpdateQuestCursor") void before_KaleidoScope_UpdateQuestCursor(PlayState* play)
{
    gPlay = play;
    gPauseCtx = &play->pauseCtx;
}

RECOMP_HOOK_RETURN("KaleidoScope_UpdateQuestCursor") void hide_ocarina_buttons() 
{
    MessageContext* msgCtx = &gPlay->msgCtx;
    u16 cursor = gPauseCtx->cursorPoint[PAUSE_QUEST];
    if (gPauseCtx->mainState == PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG)
    {
        gPauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
    }
    
    if (gPauseCtx->mainState == PAUSE_MAIN_STATE_IDLE)
    {
        if (!is_staff_cover_shown)
        {
            is_staff_cover_shown = rotate_staff_cover_closed(gPlay, gGfxCtx);
        }
        if ((CHECK_BTN_ALL(CONTROLLER1(&gPlay->state)->press.button, BTN_A)) && (msgCtx->msgLength) == 0 &&
            (cursor >= QUEST_SONG_SONATA) && (cursor <= QUEST_SONG_SUN))
        {
            Audio_PlaySfx(NA_SE_SY_DECIDE);
            gSwitchStatesTimer = 30;
            gPauseCtx->mainState = PAUSE_MAIN_STATE_SONG_PROMPT_UNUSED;
        }
    }
    else if (gPauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT_DONE)
    {
        if (DECR(gSwitchStatesTimer) == 0)
        {
            gPauseCtx->mainState = PAUSE_MAIN_STATE_SONG_PROMPT_UNUSED;
        }
        else
        {
            return;
        }
    }
    else if (gPauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT_UNUSED)
    {
        if (is_staff_cover_shown)
        {
            if (rotate_staff_cover_open(gPlay, gGfxCtx))
            {
                is_staff_cover_shown = false;
                gPauseCtx->mainState = PAUSE_MAIN_STATE_SONG_PLAYBACK_INIT;
            }
        }
        else
        {
            if (rotate_staff_cover_closed(gPlay, gGfxCtx))
            {
                is_staff_cover_shown = true;
                gPauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
            }
        }
    }
}

RECOMP_HOOK_RETURN("KaleidoScope_DrawPages")void replace_quest_texture()
{
    PlayState* play = gPlay;
    GraphicsContext* gfxCtx = gGfxCtx;

    PauseContext* pauseCtx = &play->pauseCtx;

    OPEN_DISPS(gfxCtx);

    if (pauseCtx->pageIndex == PAUSE_QUEST && pauseCtx->questPageRoll <= -314.0f)
    {
        gDPPipeSync(POLY_OPA_DISP++);

        gDPSetCombineLERP(POLY_OPA_DISP++, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, PRIMITIVE,
                            0, TEXEL0, 0, SHADE, 0);

        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 180, 180, 120, 255);

        gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_BILERP);

        Matrix_RotateYF(-3.14f, MTXMODE_NEW);
        Matrix_Translate(0.0f, sPauseMenuVerticalOffset / 100.0f, -93.0f, MTXMODE_APPLY);
        Matrix_Scale(0.78f, 0.78f, 0.78f, MTXMODE_APPLY);

        Matrix_RotateXFApply(-pauseCtx->questPageRoll / 100.0f);

        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gfxCtx);

        POLY_OPA_DISP = KaleidoScope_DrawPageSections(POLY_OPA_DISP, pauseCtx->questPageVtx, sMapPageBgTextures);
    }

    CLOSE_DISPS(gfxCtx);
}