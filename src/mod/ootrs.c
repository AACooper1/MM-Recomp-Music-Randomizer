#include "ootrs.h"

bool prepare_oot_audiotables()
{
    if (read_oot_audiobin())
    {
        AudioTableHeader soundTableHeaderTemp;
        AudioTableHeader bankTableHeaderTemp;
        get_oot_audiobin_headers(&soundTableHeaderTemp, &bankTableHeaderTemp);
        
        OoTSoundTable = recomp_alloc(sizeof(AudioTableHeader) + sizeof(AudioTableEntry) * soundTableHeaderTemp.numEntries);
        OoTBankTable = recomp_alloc(sizeof(AudioTableHeader) + sizeof(AudioTableEntry) * bankTableHeaderTemp.numEntries);

        OoTSoundTable->header = soundTableHeaderTemp;
        OoTBankTable->header = bankTableHeaderTemp;

        get_oot_audiobin_entries(OoTSoundTable->entries, OoTBankTable->entries);
    }
}

void prepare_oot_track()
{
    
}