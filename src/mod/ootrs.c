#include "ootrs.h"

bool prepare_oot_audiotables()
{
    if (read_oot_audiobin())
    {
        return true;
    }
    else
    {
        return false;
    }
}

int prepare_oot_bank(cTrack* track)
{
    return -1;
}