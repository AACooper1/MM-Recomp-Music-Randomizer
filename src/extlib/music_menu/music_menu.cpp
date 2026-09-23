#include "music_menu.hpp"

int reroll_slot_impl(int slotIdx)
{
    seed->randomize_slot(slotIdx);

    return seed->randomized[slotIdx]->databaseIndex;
}

RECOMP_DLL_FUNC(reroll_slot)
{
    int jobId = RECOMP_ARG(int, 0);
    int slotIdx = RECOMP_ARG(int, 1);

    if (!jobs.contains(jobId))
    {
        RECOMP_RETURN(int, -1);
    }

    jobs[jobId]->run(reroll_slot_impl(slotIdx));

    RECOMP_RETURN(jobs[jobId]->get_state());
}