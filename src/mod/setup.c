#include "setup.h"

RECOMP_HOOK("ConsoleLogo_Init") void get_game_state_x(GameState* thisx)
{
    gxState = thisx;
}

RECOMP_HOOK_RETURN("ConsoleLogo_Init") void start_the_um_yeah()
{
    
}

RECOMP_CALLBACK(".", music_rando_begin) void start()
{
    music_rando_update_db();
}

void music_rando_update_db()
{
    update_log_level();
    logger.debug("%s", "Mod-side logger OK!\n");
    logger.noheader.debug("%s", "Mod-side no-header logger OK!\n");

    logger.noheader.debug("Moving into setup gamestate!\n");
    gxState->main = music_rando_setup_main;
}

void music_rando_setup_main()
{
    char jobMsg[JOB_MSG_BUFFER_SIZE] = {0};

    switch (jobState)
    {
        case UNSTARTED:
            unsigned char* modPath = recomp_get_mod_folder_path();
            dbJobId = prepare_database(modPath);
            recomp_free(modPath);

            jobState = music_rando_poll_thread(dbJobId, jobMsg);
            break;
        case RUNNING:
            jobState = music_rando_poll_thread(dbJobId, jobMsg);
            break;
        case DONE:
            music_rando_cleanup_thread(dbJobId);
            init_startup_menu();
            break;
        case ERROR:
            logger.error("Job returned error!\n");
            init_startup_menu();
            break;
    }

    logger.noheader.dev("Job state is %x: %s!\n", jobState, jobMsg);
}