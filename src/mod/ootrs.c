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

        print_bytes(&logger, (void*)SEGMENT_ROM_START(Audiotable), 0x100);
    }
}

uintptr_t sample_memcmp(Sample* mmSample, char* OoTSampleData, size_t sampleSize)
{
    if (mmSample->isRelocated && mmSample)
    {
        for (int i = 0; i < sampleSize; i++)
        {
            if (OoTSampleData[i] != mmSample->sampleAddr[i])
            {
                return NULL;
            }
        }
        return (uintptr_t)mmSample->sampleAddr;
    }
    return NULL;
}

extern void* AudioLoad_SyncLoadFont(u32 fontId);

uintptr_t find_sample_in_mm_banks(char* OoTSampleData, size_t sampleSize)
{
    uintptr_t retval = 0;
    for (int mmBankNo = 0; mmBankNo < gSoundFontTable.header.numEntries; mmBankNo++)
    {
        SoundFont* mmFont = &gAudioCtx.soundFontList[mmBankNo];
        Sample* mmSample;
        AudioLoad_SyncLoadFont(mmBankNo);
        for (int drumIdx = 0; drumIdx < mmFont->numDrums; drumIdx++)
        {
            if (mmFont->drums[drumIdx])
            {
                mmSample = mmFont->drums[drumIdx]->tunedSample.sample;
                if (mmSample->isRelocated && mmSample)
                {
                    if ((retval = sample_memcmp(mmSample, OoTSampleData, sampleSize)))
                    {
                        return retval;
                    }
                }
            }
        }
        for (int instIdx = 0; instIdx < mmFont->numInstruments; instIdx++)
        {
            if (mmFont->instruments[instIdx])
            {
                mmSample = mmFont->instruments[instIdx]->lowPitchTunedSample.sample;
                if (mmSample->isRelocated && mmSample)
                {
                    if ((retval = sample_memcmp(mmSample, OoTSampleData, sampleSize)))
                    {
                        return retval;
                    }
                }
                mmSample = mmFont->instruments[instIdx]->normalPitchTunedSample.sample;
                if (mmSample->isRelocated && mmSample)
                {
                    if ((retval = sample_memcmp(mmSample, OoTSampleData, sampleSize)))
                    {
                        return retval;
                    }
                }
                mmSample = mmFont->instruments[instIdx]->highPitchTunedSample.sample;
                if (mmSample->isRelocated && mmSample)
                {
                    if ((retval = sample_memcmp(mmSample, OoTSampleData, sampleSize)))
                    {
                        return retval;
                    }
                }
            }
        }
        for (int sfxIdx = 0; sfxIdx < mmFont->numSfx; sfxIdx++)
        {
            // SoundEffects isn't a pointer to pointers, for some reason
            mmSample = mmFont->soundEffects[sfxIdx].tunedSample.sample;
            if (mmSample->isRelocated && mmSample)
            {
                if ((retval = sample_memcmp(mmSample, OoTSampleData, sampleSize)))
                {
                    return retval;
                }
            }
        }
    }

    return retval;
}

// Pretty much just take the OoTR approach here.
// For each sample in the OoT bank, scan the MM audiobanks to see if they point to a matching sample.
// If so, just change the address to that.
// Otherwise, add them to the sounds list
void link_oot_bank_to_sounds(cTrack* track)
{
    SoundFont* bankData;

    // Handle vanilla and custom banks separately, as the latter needs to be pulled from cTrack/extlib
    if (!track->hasBank)
    {
        // If already added, just update bankNo
        if (OoTBanksAddedIdx[track->bankNo])
        {
            track->bankNo = OoTBanksAddedIdx[track->bankNo];
            return;
        }
        else
        {
            uintptr_t bankDataOffset = OoTBankTable->entries[track->bankNo].romAddr;
            if (bankDataOffset > OoTBankData)
        }

    }
    else
    {
        SoundFont* OoTFont = (SoundFont*)track->bank.data;
        if (OoTFont)
        {
            for (int i = 0; i < )
        }
        else
        {
            logger.error("Called link_oot_bank_to_sounds before loading bank!");
        }
    }
}