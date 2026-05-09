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
        return true;
    }
    else
    {
        return false;
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
                return false;
            }
        }
        return (uintptr_t)mmSample->sampleAddr;
    }
    return false;
}

extern void* AudioLoad_SyncLoadFont(u32 fontId);

// Potential issue, what if a bank uses sample at offset 0x00000000?
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
                        goto ret;
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
                        goto ret;
                    }
                }
                mmSample = mmFont->instruments[instIdx]->normalPitchTunedSample.sample;
                if (mmSample->isRelocated && mmSample)
                {
                    if ((retval = sample_memcmp(mmSample, OoTSampleData, sampleSize)))
                    {
                        goto ret;
                    }
                }
                mmSample = mmFont->instruments[instIdx]->highPitchTunedSample.sample;
                if (mmSample->isRelocated && mmSample)
                {
                    if ((retval = sample_memcmp(mmSample, OoTSampleData, sampleSize)))
                    {
                        goto ret;
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
                    goto ret;
                }
            }
        }
    }

    ret:
        if (retval) logger.noheader.dev("Returning sample address: %p\n", retval);
        else logger.noheader.dev("Did not find sample.\n");
        return retval;
}

bool handle_sample_search(Sample* sample, char* sampleData, SoundFont* bankData, cTrack* track)
{
    uintptr_t mmSampleAddr;

    uintptr_t sampleDataOffset = (uintptr_t)sample->sampleAddr;
    u32 sampleDataSize = sample->size;
    sampleData = recomp_alloc(sampleDataSize);

    if (!get_oot_sound_data(sampleData, sampleDataOffset, sampleDataSize))
    {
        logger.error("Could not get OoT sample data (offset %x)!\n", sampleDataOffset);
        recomp_free(sampleData);
        return false;
    }
    // If found match in MM, update the sample (pointed to by the bank)
    if ((mmSampleAddr = find_sample_in_mm_banks(sampleData, sampleDataSize)))
    {
        sample->sampleAddr = (u8*)mmSampleAddr;
        recomp_free(sampleData);
    }
    // Otherwise, add it as a new zsound
    else
    {
        cSound* newSound = &track->sounds[track->numSounds];
        newSound->id = track->numSounds;
        newSound->size = sampleDataSize;
        newSound->sampleAddr = sampleDataOffset;
        newSound->data = sampleData;
        track->numSounds++;

        sample->sampleAddr = sampleData;
    }

    return true;
}

// Pretty much just take the OoTR approach here.
// For each sample in the OoT bank, scan the MM audiobanks to see if they point to a matching sample.
// If so, just change the address to that.
// Otherwise, add them as a custom sound

// Return values: 
//      `false`: Error
//      `true`: Add as new bank
//         > 1: Id of existing bank
int prepare_oot_bank(cTrack* track)
{
    char* sampleData;
    SoundFont* bankData;

    // Handle vanilla and custom banks separately, as the latter needs to be pulled from cTrack/extlib
    if (!track->hasBank)
    {
        // If already added, just update bankNo and return
        if (OoTBanksAddedIdx[track->bankNo])
        {
            return OoTBanksAddedIdx[track->bankNo];
        }
        else
        {
            // Otherwise, load the bank data.
            uintptr_t bankDataOffset = OoTBankTable->entries[track->bankNo].romAddr;
            int bankDataSize = OoTBankTable->entries[track->bankNo].size;
            bankData = recomp_alloc(bankDataSize);
            if (!get_oot_bank_data(bankData, bankDataOffset, bankDataSize))
            {
                logger.error("Could not get OoT bank data (offset %x)!\n", bankDataOffset);
                recomp_free(bankData);
                return false;
            }
            // Update the address in the OoT bank table
            OoTBankTable->entries[track->bankNo].romAddr = (uintptr_t)bankData;
            OoTBanksAddedIdx[track->bankNo] = true;
        }

    }
    else
    {
        bankData = (SoundFont*)track->bank.data;
        if (!bankData)
        {
            logger.error("Called link_oot_bank_to_sounds before loading bank!\n");
            return false;
        }
    }

    Sample* thisSample;
    for (int drumIdx = 0; drumIdx < bankData->numDrums; drumIdx++)
    {
        thisSample = bankData->drums[drumIdx]->tunedSample.sample;
        if (thisSample)
        {
            handle_sample_search(thisSample, sampleData, bankData, track);
        }
        else
        {
            logger.error("Drum with index 0x%x had null sampleAddr!!\n", drumIdx);
            return false;
        }
    }
    for (int instIdx = 0; instIdx < bankData->numInstruments; instIdx++)
    {
        bool found_a_sample = false;

        thisSample = bankData->instruments[instIdx]->lowPitchTunedSample.sample;
        if (thisSample)
        {
            handle_sample_search(thisSample, sampleData, bankData, track);
            found_a_sample = true;
        }

        thisSample = bankData->instruments[instIdx]->normalPitchTunedSample.sample;
        if (thisSample)
        {
            handle_sample_search(thisSample, sampleData, bankData, track);
            found_a_sample = true;
        }

        thisSample = bankData->instruments[instIdx]->highPitchTunedSample.sample;
        if (thisSample)
        {
            handle_sample_search(thisSample, sampleData, bankData, track);
            found_a_sample = true;
        }

        if (!found_a_sample)
        {
            logger.error("Instrument with index 0x%x had no samples!!\n", instIdx);
            return false;
        }
    }
    for (int sfxIdx = 0; sfxIdx < bankData->numSfx; sfxIdx++)
    {
        thisSample = bankData->soundEffects[sfxIdx].tunedSample.sample;
        if (thisSample)
        {
            handle_sample_search(thisSample, sampleData, bankData, track);
        }
        else
        {
            logger.error("Sfx with index 0x%x had null sampleAddr!!\n", sfxIdx);
            return false;
        }
    }

    logger.debug("Successfully prepared bank for OoT track %s (bankNo: %x)!\n", track->name, track->bankNo);
    return true;
}