#include "music_randomizer.h"

RECOMP_IMPORT(".", int set_log_level(int level));

RECOMP_CALLBACK("*", recomp_on_init) void init_categorySequences()
{
    logLevel = set_log_level(LOG_DEBUG);
    log_debug("Initializing category sequence table. Probably about to crash. \n");
    categorySequences = categorySequences_init();
    log_debug("CatSeq table at address %p.\n", categorySequences);
    print_bytes(categorySequences, sizeof(Vector*));

    print_bytes(categorySequences[0], sizeof(Vector));

    if (logLevel >= LOG_DEBUG)
    {
        for (int i = 0; i < 12; i++)
        {
            vec_printData(categorySequences[i]);
        }
    } 

    // Cleanup
    vec_teardown(categorySequences);
}