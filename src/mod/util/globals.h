#ifndef GLOBALS_H
#define GLOBALS_H

#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompui.h"

#include <stdbool.h>

#include "audio_api/all.h"

#include "recompuiColors.h"
#include "modtrackdefs.h"

#include "logging.h"

RECOMP_IMPORT("*", unsigned char* recomp_get_mod_folder_path());

#define NUM_SONG_SLOTS 0x80

extern Logger logger;
extern GameState* playCtx;
extern cTrack randomized[NUM_SONG_SLOTS];

#endif