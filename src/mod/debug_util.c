#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "libc64/sprintf.h"

#include "debug_util.h"

#define NUM_LINES 8
#define NUM_CHARS 32

static char print_lines[NUM_LINES][NUM_CHARS];
static u8 print_fresh[NUM_LINES];

void Debug_Print_Draw(GraphicsContext* gfxCtx) {
    GfxPrint printer;
    Gfx* gfx;
    u8 line = 0;

    OPEN_DISPS(gfxCtx);

    gfx = POLY_OPA_DISP + 1;
    gSPDisplayList(OVERLAY_DISP++, gfx);
    GfxPrint_Init(&printer);
    GfxPrint_Open(&printer, gfx);

    for (line = 0; line < NUM_LINES; line++) {
        if (print_fresh[line]) {
            GfxPrint_SetColor(&printer, 255, 0, 255, 255);
        } else {
            GfxPrint_SetColor(&printer, 200, 200, 200, 255);
        }

        GfxPrint_SetPos(&printer, 1, 7 + line);
        GfxPrint_Printf(&printer, &print_lines[line]);

        print_fresh[line] = false;
    }

    gfx = GfxPrint_Close(&printer);
    GfxPrint_Destroy(&printer);
    gSPEndDisplayList(gfx++);
    gSPBranchList(POLY_OPA_DISP, gfx);
    POLY_OPA_DISP = gfx;

    CLOSE_DISPS(gfxCtx);
}

void Debug_Print(u8 line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (line > NUM_LINES) {
        return;
    }

    vsprintf(&print_lines[line], fmt, args);
    print_fresh[line] = true;
}

RECOMP_HOOK("Play_PostWorldDraw") void on_Play_PostWorldDraw(PlayState* this) {
    Debug_Print_Draw(this->state.gfxCtx);
}