// MM Recomp Mod Requirements
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "logging.h"

Logger logger;

RECOMP_IMPORT("*", unsigned char* recomp_get_mod_folder_path());
RECOMP_IMPORT(".", int launch_tests(char* savePath));

RECOMP_HOOK_RETURN("AudioLoad_Init") void run()
{
    unsigned char* savePath = recomp_get_mod_folder_path();

    recomp_printf("%p\n", &gAudioCtx.sequenceTable);
    recomp_printf("%p\n", gAudioCtx.sequenceTable);
    recomp_printf("%i\n", gAudioCtx.sequenceTable->header.numEntries);

    launch_tests(savePath);
}