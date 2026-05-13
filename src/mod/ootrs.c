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
    logger.noheader.dev("In sample_memcmp now\n");
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
    logger.dev("In find_sample_in_mm_banks\n");
    uintptr_t retval = 0;
    for (int mmBankNo = 3; mmBankNo < gSoundFontTable.header.numEntries; mmBankNo++)
    {
        SoundFont* mmFont = &gAudioCtx.soundFontList[mmBankNo];
        Sample* mmSample;
        void* fontData = AudioLoad_SyncLoadFont(mmBankNo);
        logger.dev("Loaded font %x.\n", mmBankNo);
        print_bytes(&logger, fontData, 0x100);
        for (int drumIdx = 0; drumIdx < mmFont->numDrums; drumIdx++)
        {
            logger.noheader.dev("numDrums OK\n");
            if (mmFont->drums[drumIdx])
            {
                logger.noheader.dev("mmFont drum OK\n");
                mmSample = mmFont->drums[drumIdx]->tunedSample.sample;
                logger.noheader.dev("mmSample OK\n");
                print_bytes(&logger, mmSample, sizeof(Sample));
                if (mmSample->isRelocated && mmSample)
                {
                    logger.noheader.dev("Drum %x is relocated.\n", drumIdx);
                    if ((retval = sample_memcmp(mmSample, OoTSampleData, sampleSize)))
                    {
                        logger.debug("Found sample at MM bank %x drum %x!\n", mmBankNo, drumIdx);
                        goto ret;
                    }
                }
                else
                {
                    logger.noheader.dev("Failure: mmSample = %p; isRelocated = %x\n", mmSample, mmSample->isRelocated);
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
                        logger.dev("Found sample at MM bank %x inst %x!\n", mmBankNo, instIdx);
                        goto ret;
                    }
                }
                mmSample = mmFont->instruments[instIdx]->normalPitchTunedSample.sample;
                if (mmSample->isRelocated && mmSample)
                {
                    if ((retval = sample_memcmp(mmSample, OoTSampleData, sampleSize)))
                    {
                        logger.dev("Found sample at MM bank %x inst %x!\n", mmBankNo, instIdx);
                        goto ret;
                    }
                }
                mmSample = mmFont->instruments[instIdx]->highPitchTunedSample.sample;
                if (mmSample->isRelocated && mmSample)
                {
                    if ((retval = sample_memcmp(mmSample, OoTSampleData, sampleSize)))
                    {
                        logger.dev("Found sample at MM bank %x inst %x!\n", mmBankNo, instIdx);
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
                    logger.dev("Found sample at MM bank %x sfx %x!\n", mmBankNo, sfxIdx);
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
    logger.noheader.dev("(In handle_sample_search now)\n");
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
    else
    {
        logger.noheader.debug("Pulled sample data from Audiotable file successfully. Begins %02x.\n", sampleData[0]);
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
        logger.dev("Did not find sample in OoT banks, will be added as zsound.\n");
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
// 
// Return values: 
//      `false`: Error
//      `true`: Add as new bank
//         > 1: Id of existing bank
int prepare_oot_bank(cTrack* track)
{
    char* sampleData;
    char* bankData;

    SoundFont bankHeader;
    AudioTableEntry* bankEntry = &OoTBankTable->entries[track->bankNo];
    bankHeader.sampleBankId1 = (bankEntry->shortData1 & 0xFF00) >> 8;
    bankHeader.sampleBankId1 = (bankEntry->shortData1 & 0xFF);
    bankHeader.numInstruments = (bankEntry->shortData2 & 0xFF00) >> 8;
    bankHeader.numDrums = (bankEntry->shortData2 & 0xFF);
    bankHeader.numSfx = (bankEntry->shortData3);

    // Handle vanilla and custom banks separately, as the latter needs to be pulled from cTrack/extlib
    if (!track->hasBank)
    {
        // If already added, just update bankNo and return
        if (OoTBanksAddedIdx[track->bankNo])
        {
            logger.dev("Found existing OoT bank for %x, %x.\n", track->bankNo, OoTBanksAddedIdx[track->bankNo]);
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
            else
            {
                logger.noheader.dev("Got OoT bank %x from extlib (offset %p)!\n", track->bankNo, bankDataOffset);
                print_bytes(&logger, bankData, bankDataSize);

                bankHeader.drums = bankData + ((uintptr_t*)bankData)[0];
                logger.noheader.dev("Drums are at offset %p.\n", ((uintptr_t*)bankData)[0]);
                print_bytes(&logger,  bankData + ((uintptr_t*)bankData)[0], sizeof(Drum*) * bankHeader.numDrums);

                bankHeader.instruments = bankData + ((uintptr_t*)bankData)[1];
                logger.noheader.dev("Instruments are at offset %p.\n", ((uintptr_t*)bankData)[1]);

                bankHeader.soundEffects = bankData + ((uintptr_t*)bankData)[2];
                logger.noheader.dev("Sfx are at offset %p.\n", ((uintptr_t*)bankData)[2]);
            }
            // Update the address in the OoT bank table
            OoTBankTable->entries[track->bankNo].romAddr = (uintptr_t)bankData;
            OoTBanksAddedIdx[track->bankNo] = true;
        }

    }
    else
    {
        logger.noheader.dev("Track has custom bank, adding...\n");
        bankData = track->bank.data;
        if (!bankData)
        {
            logger.error("Called link_oot_bank_to_sounds before loading bank!\n");
            return false;
        }
    }

    Sample* thisSample;
    logger.dev("OoT Bank with idx %x has %x drums, %x insts, %x sfx.\n", track->bankNo, bankHeader.numDrums, bankHeader.numInstruments, bankHeader.numSfx);
    for (int drumIdx = 0; drumIdx < bankHeader.numDrums; drumIdx++)
    {
        Drum* thisDrum = bankData + (uintptr_t)bankHeader.drums[drumIdx];
        logger.noheader.dev("Drum address is %p (offset %p)\n", thisDrum, bankHeader.drums[drumIdx]);
        thisSample = bankData + (uintptr_t)thisDrum->tunedSample.sample;
        if (thisSample)
        {
            logger.noheader.dev("Handling drum with idx %x in OoT bank %x...\n", drumIdx, track->bankNo);
            print_bytes(&logger, thisDrum, sizeof(Drum));
            logger.noheader.dev("Sample struct is at %p (offset %p)\n", thisSample, thisDrum->tunedSample.sample);
            print_bytes(&logger, thisSample, sizeof(Sample));
            logger.dev("Sample address is %p.\n", thisSample->sampleAddr);
            handle_sample_search(thisSample, sampleData, bankData, track);
        }
        else
        {
            logger.error("Drum with index 0x%x had null sampleAddr!!\n", drumIdx);
            return false;
        }
    }
    for (int instIdx = 0; instIdx < bankHeader.numInstruments; instIdx++)
    {
        bool found_a_sample = false;
        logger.noheader.dev("Handling inst with idx %x in OoT bank %x...\n", instIdx, track->bankNo);
        print_bytes(&logger, &bankHeader.instruments, sizeof(Instrument*) * bankHeader.numInstruments);
        thisSample = bankHeader.instruments[instIdx]->lowPitchTunedSample.sample;
        logger.noheader.dev("thisSample is at %p", thisSample);
        if (thisSample)
        {
            handle_sample_search(thisSample, sampleData, bankData, track);
            found_a_sample = true;
        }

        thisSample = bankHeader.instruments[instIdx]->normalPitchTunedSample.sample;
        if (thisSample)
        {
            handle_sample_search(thisSample, sampleData, bankData, track);
            found_a_sample = true;
        }

        thisSample = bankHeader.instruments[instIdx]->highPitchTunedSample.sample;
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
    for (int sfxIdx = 0; sfxIdx < bankHeader.numSfx; sfxIdx++)
    {
        thisSample = bankHeader.soundEffects[sfxIdx].tunedSample.sample;
        logger.noheader.dev("Handling sfx with idx %x in OoT bank %x...\n", sfxIdx, track->bankNo);
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

    logger.debug("Successfully prepared bank for OoT track %s (bankNo %x = MM bank %x)!\n", track->name, track->bankNo, OoTBanksAddedIdx[track->bankNo]);
    return true;
}