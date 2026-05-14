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
bool should_stop;
uintptr_t sample_memcmp(u8* mmSample, u8* OoTSampleData, size_t sampleSize)
{
    // logger.noheader.dev("\tmmSampleAddr : %p (begins %02x %02x %02x %02x)\n", mmSample, mmSample[0], mmSample[1], mmSample[2], mmSample[3]);
    // logger.noheader.dev("\tOoTSampleAddr: %p (begins %02x %02x %02x %02x)\n", OoTSampleData, OoTSampleData[0], OoTSampleData[1], OoTSampleData[2], OoTSampleData[3]);

    if (OoTSampleData[6] == 0x1C && OoTSampleData[7] == 0x4E)
    {
        if (should_stop)
        {
            print_bytes(&logger, OoTSampleData, 0x10);
            print_bytes(&logger, mmSample, 0x10);
            while (true) { }
        }
    }
    for (int i = 0; i < sampleSize; i++)
    {
        if (OoTSampleData[i] != mmSample[i])
        {
            logger.noheader.dev("No match.\n");
            return false;
        }
    }
    logger.noheader.dev("Found match, returning!\n");
    return (uintptr_t)mmSample;
}

extern void* AudioLoad_SyncLoadFont(u32 fontId);
extern void AudioLoad_SyncDma(uintptr_t devAddr, u8* ramAddr, size_t size, s32 medium);

// Potential issue, what if a bank uses sample at offset 0x00000000?
uintptr_t find_sample_in_mm_banks(u8* OoTSampleData, size_t sampleSize)
{
    uintptr_t retval = 0;
    for (int mmBankNo = 3; mmBankNo < gSoundFontTable.header.numEntries; mmBankNo++)
    {
        SoundFont* mmFont = &gAudioCtx.soundFontList[mmBankNo];
        Sample* mmSample;
        u8* sampleData;
        void* fontData = AudioLoad_SyncLoadFont(mmBankNo);

        logger.dev("Loaded font %x. \n", mmBankNo, fontData);
        for (int drumIdx = 0; drumIdx < mmFont->numDrums; drumIdx++)
        {
            if (mmFont->drums[drumIdx])
            {
                logger.noheader.dev("Comparing OoT sample with MM bank %x drum %i of %i...", mmBankNo, drumIdx, mmFont->numDrums - 1);
                mmSample = mmFont->drums[drumIdx]->tunedSample.sample;

                sampleData = recomp_alloc(mmSample->size);
                AudioLoad_SyncDma((uintptr_t)mmSample->sampleAddr, sampleData, mmSample->size, mmSample->medium);
                logger.noheader.dev(" Successfully loaded sample. Comparing... ");
                
                mmSample->isRelocated = true;
                if (mmSample->isRelocated && mmSample)
                {
                    if ((retval = sample_memcmp(sampleData, OoTSampleData, sampleSize)))
                    {
                        goto ret;
                    }
                }
                else
                {
                    logger.noheader.dev("Failure: mmSample = %p; isRelocated = %x\n", mmSample, mmSample->isRelocated);
                }
                recomp_free(sampleData);
            }
        }
        for (int instrumentIdx = 0; instrumentIdx < mmFont->numInstruments; instrumentIdx++)
        {
            if (mmFont->instruments[instrumentIdx])
            {
                logger.noheader.dev("Comparing OoT sample with MM bank %x instrument %i of %i...", mmBankNo, instrumentIdx, mmFont->numInstruments - 1);
                mmSample = mmFont->instruments[instrumentIdx]->highPitchTunedSample.sample;
                if (mmSample)
                {
                    sampleData = recomp_alloc(mmSample->size);
                    AudioLoad_SyncDma((uintptr_t)mmSample->sampleAddr, sampleData, mmSample->size, mmSample->medium);
                    logger.noheader.dev(" Successfully loaded high-pitch tuned sample. Comparing... ");
                    
                    mmSample->isRelocated = true;
                    if (mmSample->isRelocated && mmSample)
                    {
                        if ((retval = sample_memcmp(sampleData, OoTSampleData, sampleSize)))
                        {
                            goto ret;
                        }
                    }
                    else
                    {
                        logger.noheader.dev("Failure: mmSample = %p; isRelocated = %x\n", mmSample, mmSample->isRelocated);
                    }
                    recomp_free(sampleData);
                }
                mmSample = mmFont->instruments[instrumentIdx]->normalPitchTunedSample.sample;
                if (mmSample)
                {
                    if (instrumentIdx == 7 && mmBankNo == 3) should_stop = true;
                    sampleData = recomp_alloc(mmSample->size);
                    AudioLoad_SyncDma((uintptr_t)mmSample->sampleAddr, sampleData, mmSample->size, mmSample->medium);
                    logger.noheader.dev(" Successfully loaded normal-pitch tuned sample. Comparing... ");
                    
                    mmSample->isRelocated = true;
                    if (mmSample->isRelocated && mmSample)
                    {
                        if ((retval = sample_memcmp(sampleData, OoTSampleData, sampleSize)))
                        {
                            goto ret;
                        }
                    }
                    else
                    {
                        logger.noheader.dev("Failure: mmSample = %p; isRelocated = %x\n", mmSample, mmSample->isRelocated);
                    }
                    recomp_free(sampleData);
                    should_stop = false;
                }
                mmSample = mmFont->instruments[instrumentIdx]->lowPitchTunedSample.sample;
                if (mmSample)
                {
                    sampleData = recomp_alloc(mmSample->size);
                    AudioLoad_SyncDma((uintptr_t)mmSample->sampleAddr, sampleData, mmSample->size, mmSample->medium);
                    logger.noheader.dev(" Successfully loaded low-pitch tuned sample. Comparing... ");
                    
                    mmSample->isRelocated = true;
                    if (mmSample->isRelocated && mmSample)
                    {
                        if ((retval = sample_memcmp(sampleData, OoTSampleData, sampleSize)))
                        {
                            goto ret;
                        }
                    }
                    else
                    {
                        logger.noheader.dev("Failure: mmSample = %p; isRelocated = %x\n", mmSample, mmSample->isRelocated);
                    }
                    recomp_free(sampleData);
                }
            }
        }
        for (int sfxIdx = 0; sfxIdx < mmFont->numSfx; sfxIdx++)
        {
            logger.noheader.dev("Comparing OoT sample with MM bank %x sound effect %i of %i...", mmBankNo, sfxIdx, mmFont->numSfx - 1);
            mmSample = mmFont->drums[sfxIdx]->tunedSample.sample;

            if (mmSample)
            {
                sampleData = recomp_alloc(mmSample->size);
                AudioLoad_SyncDma((uintptr_t)mmSample->sampleAddr, sampleData, mmSample->size, mmSample->medium);
            logger.noheader.dev(" Successfully loaded sample. Comparing... ");
            }
            else
            {
                continue;
            }
            
            mmSample->isRelocated = true;
            if (mmSample->isRelocated)
            {
                if ((retval = sample_memcmp(sampleData, OoTSampleData, sampleSize)))
                {
                    goto ret;
                }
            }
            else
            {
                logger.noheader.dev("Failure: mmSample = %p; isRelocated = %x\n", mmSample, mmSample->isRelocated);
            }
            recomp_free(sampleData);
        }
    }

    ret:
        if (retval) logger.noheader.dev("Returning sample address: %p\n", retval);
        else logger.noheader.dev("Did not find sample.\n");
        return retval;
}

bool handle_sample_search(Sample* sample, u8* sampleData, SoundFont* bankData, cTrack* track)
{
    logger.noheader.dev("(In handle_sample_search now)\n");
    uintptr_t mmSampleAddr;

    uintptr_t sampleDataOffset = (uintptr_t)sample->sampleAddr;
    if (sampleDataOffset > AUDIO_RELOCATED_ADDRESS_START)
    {
        // Already handled
        logger.noheader.dev("Sample is already loaded.\n");
        return true;
    }
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
        logger.noheader.debug("Pulled sample data from Audiotable file successfully. Offset was %p, begins %02x %02x %02x %02x.\n", sampleDataOffset, sampleData[0], sampleData[1], sampleData[2], sampleData[3]);
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
    u8* sampleData;
    u8* bankData;

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
                bankHeader.drums = bankData + ((uintptr_t*)bankData)[0];
                logger.noheader.dev("Drums are at offset %p.\n", ((uintptr_t*)bankData)[0]);

                bankHeader.instruments = bankData + ((uintptr_t*)bankData)[1];
                logger.noheader.dev("Instruments are at offset %p.\n", ((uintptr_t*)bankData)[1]);

                bankHeader.soundEffects = bankData + ((uintptr_t*)bankData)[2];
                logger.noheader.dev("Sfx are at offset %p.\n", ((uintptr_t*)bankData)[2]);
            }
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
        thisSample = bankData + (uintptr_t)thisDrum->tunedSample.sample;
        if (thisSample)
        {
            logger.dev("Searching for OoT bank %x drum %x.\n", track->bankNo, drumIdx);
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
        bool found_sample = false;
        Instrument* thisInst = bankData + (uintptr_t)bankHeader.instruments[instIdx];
        thisSample = bankData + (uintptr_t)thisInst->highPitchTunedSample.sample;
        if (thisSample)
        {
            logger.dev("Searching for OoT bank %x inst %x (high).\n", track->bankNo, instIdx);
            handle_sample_search(thisSample, sampleData, bankData, track);
            found_sample = true;
        }
        thisSample = bankData + (uintptr_t)thisInst->normalPitchTunedSample.sample;
        if (thisSample)
        {
            logger.dev("Searching for OoT bank %x inst %x (normal).\n", track->bankNo, instIdx);
            handle_sample_search(thisSample, sampleData, bankData, track);
            found_sample = true;
        }
        thisSample = bankData + (uintptr_t)thisInst->lowPitchTunedSample.sample;
        if (thisSample)
        {
            logger.dev("Searching for OoT bank %x inst %x (low).\n", track->bankNo, instIdx);
            handle_sample_search(thisSample, sampleData, bankData, track);
            found_sample = true;
        }

        if (!found_sample)
        {
            logger.error("Instrument with index 0x%x had no sampleAddrs!!\n", instIdx);
            return false;
        }
    }
    for (int sfxIdx = 0; sfxIdx < bankHeader.numSfx; sfxIdx++)
    {
        SoundEffect* thisSfx = &bankHeader.soundEffects[sfxIdx];
        thisSample = bankData + (uintptr_t)thisSfx->tunedSample.sample;
        if (thisSample)
        {
            logger.dev("Searching for OoT bank %x sfx %x.\n", track->bankNo, sfxIdx);
            handle_sample_search(thisSample, sampleData, bankData, track);
        }
        else
        {
            logger.error("Sfx with index 0x%x had null sampleAddr!!\n", sfxIdx);
            return false;
        }
    }

    logger.debug("Successfully prepared bank for OoT track %s (bankNo %x)!\n", track->name, track->bankNo, OoTBanksAddedIdx[track->bankNo]);
    // Mark as loaded in the OoT bank table. Index will be added in main.c
    OoTBankTable->entries[track->bankNo].romAddr = (uintptr_t)bankData;
    OoTBanksAddedIdx[track->bankNo] = true;
    return true;
}