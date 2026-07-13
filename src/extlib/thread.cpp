#include "thread.hpp"

std::unordered_map<int, std::unique_ptr<MusicRandoJob>> jobs;

int test_async()
{
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}

RECOMP_DLL_FUNC(music_rando_poll_thread)
{
    int jobId = RECOMP_ARG(int, 0);

    ThreadState state = jobs[jobId]->poll();

    RECOMP_RETURN(int, state);
}

int music_rando_create_thread(std::function<int()> func)
{
    int jobId = 0;
    while(jobs.contains(jobId))
    {
        jobId++;
    }

    jobs.emplace(jobId, std::make_unique<MusicRandoJob>());

    jobs[jobId]->start(func);

    return jobId;
}
