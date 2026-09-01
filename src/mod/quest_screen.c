#include "quest_screen.h"

extern f32 sPauseMenuVerticalOffset;
extern TexturePtr sMapPageBgTextures[];
PlayState* gPlay;
GraphicsContext* gGfxCtx;

extern Gfx* KaleidoScope_DrawPageSections(Gfx* gfx, Vtx* vertices, TexturePtr* textures);

RECOMP_HOOK("KaleidoScope_DrawPages") void Pre_replace_quest_texture(PlayState* play, GraphicsContext* gfxCtx)
{
    gPlay = play;
    gGfxCtx = gfxCtx;
}

RECOMP_HOOK_RETURN("KaleidoScope_DrawPages") void cover_staff_texture()
{
    PlayState* play = gPlay;
    GraphicsContext* gfxCtx = gGfxCtx;

    PauseContext* pauseCtx = &play->pauseCtx;

    OPEN_DISPS(gfxCtx);
    if (pauseCtx->pageIndex == PAUSE_QUEST)
    {

        // Draw the cover over the music staff
        Vtx* vertices0 = &pauseCtx->questVtx[QUEST_SKULL_TOKEN * 4];
        gDPPipeSync(POLY_OPA_DISP++);
        
        gDPSetCombineLERP(POLY_OPA_DISP++, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, PRIMITIVE, 0,
                                TEXEL0, 0, SHADE, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 180, 180, 120, 255);
        gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_BILERP);
        
        gDPLoadTextureBlock(POLY_OPA_DISP++, gQuestPageStaffCoverTex, G_IM_FMT_IA, G_IM_SIZ_8b, 128, 32, 0, G_TX_NOMIRROR | G_TX_WRAP,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

        gSPVertex(POLY_OPA_DISP++, vertices0, 4, 0);

        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    }
    CLOSE_DISPS();
}

GraphicsContext* gGfxCtx;
RECOMP_HOOK("KaleidoScope_DrawPages") void draw_pixel_grid(PlayState* this, GraphicsContext* gfxCtx)
{
    gGfxCtx = this->state.gfxCtx;
}

extern s16 sQuestVtxRectLeft[];
extern s16 sQuestVtxRectTop[];
extern s16 sQuestVtxWidths[];
extern s16 sQuestVtxHeights[];

RECOMP_HOOK("KaleidoScope_SetVertices") void music_buttons_init(PlayState* play, GraphicsContext* gfxCtx)
{
    sQuestVtxRectLeft[QUEST_SKULL_TOKEN] = -118;
    sQuestVtxRectTop[QUEST_SKULL_TOKEN] = -39;
    sQuestVtxWidths[QUEST_SKULL_TOKEN] = 128;
    sQuestVtxHeights[QUEST_SKULL_TOKEN] = 32;
}