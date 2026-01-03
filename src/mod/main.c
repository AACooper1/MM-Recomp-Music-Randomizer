// MM Recomp Mod Requirements
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "logging.h"

RECOMP_IMPORT("*", unsigned char* recomp_get_mod_folder_path());
RECOMP_IMPORT(".", int update_database(char* savePath));

struct Logger logger;

RECOMP_HOOK("ConsoleLogo_Init") void test_create_db()
{
    logger_init(&logger);
    logger.debug("And the %c side of it works too!\n\n", 67);

    unsigned char* savePath = recomp_get_mod_folder_path();

    int rc = update_database(savePath);
    if (rc) { logger.error ("Could not update database, aborting music rando. Error code: %i", rc); return; }
}