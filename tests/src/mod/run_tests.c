// MM Recomp Mod Requirements
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "logging.h"

struct Logger logger;

RECOMP_IMPORT("*", unsigned char* recomp_get_mod_folder_path());
RECOMP_IMPORT(".", int launch_tests(char* savePath))

RECOMP_HOOK("ConsoleLogo_Init") void run()
{
    unsigned char* savePath = recomp_get_mod_folder_path();
    launch_tests(savePath);
}