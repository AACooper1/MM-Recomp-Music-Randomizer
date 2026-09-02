#include "thread.hpp"

std::unordered_map<int, std::unique_ptr<MusicRandoJob>> jobs;

RECOMP_DLL_FUNC(music_rando_poll_thread)
{
    int jobId = RECOMP_ARG(int, 0);
    char* msg = RECOMP_ARG(char*, 1);

    if (jobs.contains(jobId))
    {
        ThreadState state = jobs[jobId]->poll(msg);
        RECOMP_RETURN(int, state);
    }
    else
    {
        RECOMP_RETURN(int, ThreadState::ERROR);
    }
}

RECOMP_DLL_FUNC(music_rando_send_thread_msg)
{
    int jobId = RECOMP_ARG(int, 0);
    ThreadState state = ThreadState(RECOMP_ARG(int, 1));

    if (jobs.contains(jobId))
    {
        jobs[jobId]->set_state(state);
        RECOMP_RETURN(int, jobs[jobId]->get_state());
    }
    else
    {
        logger.critical << "Thread with job ID " << jobId << " not found!! Aborting database read." << std::endl;
        RECOMP_RETURN(int, ThreadState::FATAL);
    }

}

RECOMP_DLL_FUNC(music_rando_cleanup_thread)
{
    int jobId = RECOMP_ARG(int, 0);
    if (jobs.contains(jobId))
    {
        jobs.erase(jobId); // Calls the destructor so the thread is joined
        logger.dev.disable_header();
        logger.dev << "Tore down thread!" << std::endl;
        logger.dev.enable_header();
    }
    else
    {
        logger.dev << "Thread " << jobId << " did not exist!" << std::endl;
    }
}