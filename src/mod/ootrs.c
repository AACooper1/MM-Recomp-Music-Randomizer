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

// Pretty much just take the OoTR approach here.
// For each sample in the OoT bank, scan the MM audiobanks to see if they point to a matching sample.
// If so, just change the address to that.
// Otherwise, add them to the sounds list
void link_oot_bank_to_sounds(cTrack* track)
{
    // If vanilla bank, check if this bank has already been added and just update it to that idx if so
    if (!track->hasBank && track->bankNo)
    {
        if (OoTBanksAddedIdx[track->bankNo])
        {
            track->bankNo = OoTBanksAddedIdx[track->bankNo];
        }
    }
    for (int mmBankNo = 0; mmBankNo < gSoundFontTable.header.numEntries; mmBankNo++)
    {
        SoundFont* mmFont = &gAudioCtx.soundFontList[mmBankNo];
        for(int drumIdx = 0; drumIdx < mmFont->numDrums; drumIdx++)
        {
            Drum* mmDrum = mmFont->drums[drumIdx];
            if (mmDrum && mmDrum->tunedSample.sample)
            {
                
            }
        }
    }
}