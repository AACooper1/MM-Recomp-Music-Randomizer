#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompui.h"

#include "logging.h"
#include "modtrackdefs.h"

RECOMP_IMPORT(".", int music_rando_poll_thread(int jobId));

RECOMP_IMPORT("*", unsigned char* recomp_get_mod_folder_path());
RECOMP_IMPORT(".", int prepare_database(unsigned char* modPath));

RECOMP_DECLARE_EVENT(init_startup_menu());

typedef enum MusicRandoThreadState_t {
    UNSTARTED,
    RUNNING,
    DONE,
    ERROR
} MusicRandoThreadState;

GameState* gxState;
extern Logger logger;

MusicRandoThreadState jobState = UNSTARTED;
int dbJobId = 0;
char msg[256];

void music_rando_setup_main();
void music_rando_update_db();