#include "music_randomizer.h"

RECOMP_IMPORT(".", int set_log_level(int level));
RECOMP_IMPORT("magemods_audio_api", void AudioApi_ReplaceSequence(s32 seqId, AudioTableEntry* entry));
RECOMP_IMPORT("magemods_audio_api", void AudioApi_ReplaceSequenceFont(s32 seqId, s32 fontNum, s32 fontId));

RECOMP_DECLARE_EVENT(music_rando_on_init());

extern s32 AudioApi_GetSequenceFont(s32 seqId, s32 fontNum);

// Initializes the categories table and populates them with vanilla sequences.
void init_vanilla_sequence_categories()
{
    categorySequences = init_catSeq_table();

    for (size_t i = 0; i < 128; i++)
    {
        vec_push_back(songNames, vanillaSongNames[i]);
        char* thisSongName = (char*)recomp_alloc(sizeof(char) * 256);
        Lib_MemSet(thisSongName, 0, 256);
        vec_at(songNames, i, thisSongName);
    }
    for (int i = 128; i < 256; i++)
    {
        vec_push_back(songNames, vanillaSongNames[0]);
    }
    log_debug("End of init_vanilla_sequence_categories()\n");
}

// Add all custom sequences to the categories table.
void add_custom_sequence_categories(MMRS* usedMmrs, int numMmrs)
{
    for (int i = 0; i < numMmrs; i++)
    {
        log_debug("%i ", i)
        // Custom music tracks start at index 256 in Mage's API. Subject to change.
        int seqId = i + 0x100;
        // Offset to account for 0xXFE and 0xXFF being reserved
        if (seqId % 0xFE <= 1)
        {
            vec_push_back(songNames, "Unused (??)");
            seqId++;
        }

        // seqId += ((seqId - 256)/254) * 2;

        vec_push_back(songNames, usedMmrs[i].songName);
        for (int c = 0; c < 512; c++)
        {
            if (usedMmrs[i].categories[c])
            {
                vec_push_back(categorySequences[c], &seqId);
            }
        }
    }
}

RECOMP_CALLBACK(".", mmrs_reader_done) void init_music_rando(MMRS* usedMmrs, int numMmrs)
{
    songNames = vec_init(256);
    init_vanilla_sequence_categories();

    // Rand_Seed(get_current_time());
    // print_bytes(gAudioCtx.sequenceFontTable, (sizeof(u16) * gAudioCtx.sequenceTable->header.numEntries) + gAudioCtx.sequenceTable->header.numEntries * 20);

    add_custom_sequence_categories(usedMmrs, numMmrs);

    sequenceFontTableImpostor = recomp_alloc((sizeof(u16) * gAudioCtx.sequenceTable->header.numEntries) + gAudioCtx.sequenceTable->header.numEntries * 20);
    Lib_MemCpy(sequenceFontTableImpostor, gAudioCtx.sequenceFontTable, (sizeof(u16) * gAudioCtx.sequenceTable->header.numEntries) + gAudioCtx.sequenceTable->header.numEntries * 20);
    
    sequenceTableImpostor = recomp_alloc(sizeof(AudioTable) + sizeof(AudioTableEntry) * (numMmrs - 1));

    // print_bytes(gAudioCtx.sequenceFontTable, gAudioCtx.sequenceTable->header.numEntries * 5);

    log_debug("Copying AudioTable... ")
    
    // Header
    AudioTableHeader copyHeader = {
        /*   numEntries   */ gAudioCtx.sequenceTable->header.numEntries,
        /* unkMediumParam */ 0,
        /*    romAddr     */ (uintptr_t)recomp_alloc(sizeof(AudioTableEntry) * gAudioCtx.sequenceTable->header.numEntries),
        /*      pad       */ {0, 0, 0, 0, 0, 0, 0, 0}
    };
    sequenceTableImpostor->header = copyHeader;

    // Entries
    for (int i = 0; i < gAudioCtx.sequenceTable->header.numEntries; i++)
    {
        AudioTableEntry* thisEntry = &(gAudioCtx.sequenceTable->entries[i]);

        // Don't need to copy zseqs themselves, their pointers should remain unchanged

        AudioTableEntry copyEntry = {
            thisEntry->romAddr,
            thisEntry->size,
            thisEntry->medium,
            thisEntry->cachePolicy,
            thisEntry->shortData1,
            thisEntry->shortData2,
            thisEntry->shortData3
        };

        sequenceTableImpostor->entries[i] = copyEntry;
    }

    // if (logLevel >= LOG_DEBUG)
    // {
    //     log_debug("Copied!\n");
    //     print_bytes(&(sequenceTableImpostor.entries[2]), sizeof(AudioTableEntry));
    // }

    music_rando_on_init();
}

RECOMP_CALLBACK(".", music_rando_begin) void randomize_music()
{    
    unsigned char* save_file_path_temp = recomp_get_save_file_path();
    unsigned char* save_file_path = recomp_alloc(strlen(save_file_path_temp));
    strcpy(save_file_path, save_file_path_temp);
    recomp_free(save_file_path_temp);
    
    save_file_path+=strlen(save_file_path) - 15;

    int randSeed = fucking_use_stoi(save_file_path);
    log_info("Randomizing music for seed %i...\n\n", randSeed);

    Rand_Seed((u32)randSeed);

    for (int i = 0; i < 139; i++)
    {
        vec_randomize(categorySequences[i]);
    }
    log_debug("Passed catSeq randomization...\n")

    bool alreadyRolled[gAudioCtx.sequenceTable->header.numEntries];
    for (int i = 0; i < gAudioCtx.sequenceTable->header.numEntries; i++)
    {
        alreadyRolled[i] = false;
    }
    log_debug("Initialized alreadyRolled table...\n")

    randomizedIds = recomp_alloc(sizeof(int) * gAudioCtx.sequenceTable->header.numEntries);
    for (int z = 0; z < gAudioCtx.sequenceTable->header.numEntries; z++)
    {
        randomizedIds[z] = z;
    }
    log_debug("Initialized randomizedIds...\n")

    Vector* randomOrder = vec_init(sizeof(int));

    for (int i = 2; i < 128; i++)
    {
        vec_push_back(randomOrder, &i);
    }
    vec_randomize(randomOrder);
    log_debug("Initialized randomOrder...\n")

    if (logLevel >= LOG_DEBUG)
    {
        log_debug("\nsequenceCategories for Termina Field (id 2):")
        // vec_printData(sequenceCategories[2]);
    }

    while (randomOrder->numElements > 0)
    {
        int i = 0;
        vec_pop_back(randomOrder, &i);
        Vector* availableSeqs = vec_init(sizeof(int));

        // Get the pool of sequences to pull from
        AudioTableEntry* thisEntry = &(sequenceTableImpostor->entries[i]);
        for (u32 c = 0; c < sequenceCategories[i]->numElements; c++)
        {
            int rc;
            if ((rc = vec_concat(availableSeqs, categorySequences[((int*)sequenceCategories[i]->dataStart)[c]])) != VEC_SUCCESS)
            {
                log_error("Error with vector concat (index %i) ", i);
                vec_errmsg(rc);
                vec_printData(availableSeqs);
            }
        }
        vec_randomize(availableSeqs);

        if (logLevel >= LOG_DEBUG)
        {
            // log_debug("\nAvailable sequences for song %i (%s):", i, vanillaSongNames[i]);
            // vec_printData(availableSeqs);
        }


        // Now just vec_pop_back and replace the thing.
        int newSeqId;
        int rc;
        if (availableSeqs->numElements > 1)
        {
            while (availableSeqs->numElements > 0)
            {
                rc = vec_pop_back(availableSeqs, &newSeqId);
                if (
                    newSeqId == NA_BGM_FAIRY_FOUNTAIN ||
                    newSeqId == NA_BGM_CLOCK_TOWN_DAY_2_PTR ||
                    newSeqId == NA_BGM_MILK_BAR_DUPLICATE ||
                    newSeqId == NA_BGM_MAJORAS_LAIR ||
                    newSeqId == NA_BGM_OCARINA_LULLABY_INTRO_PTR
                )
                {
                    alreadyRolled[newSeqId] = true;
                    log_debug("Rolled 0x%02x for song %i, which is a pointer. Rerolling...", newSeqId, i);
                    continue;
                }
                // 15% chance of reroll if not custom song (possibly tweak later)
                if (newSeqId < 256)
                {
                    if (Rand_Next() % 100 < 15)
                    {
                        log_debug("Rolled 0x%02x, a vanilla track, for song %i. Rerolling...\n", newSeqId, i);
                        continue;
                    }
                }
                // Reroll if already rolled and still has seqs available.
                if (alreadyRolled[newSeqId] == false)
                {
                    // Do not mark already rolled on seldom-heard music
                    if  (!(
                    i == 0x03 ||    // Chase
                    i == 0x04 ||    // Majora's Theme
                    i == 0x05 ||    // Clock Tower
                    i == 0x0F ||    // Sharp's Curse
                    i == 0x18 ||    // File Select (Fairy's Fountain)
                    i == 0x1E ||    // Opening
                    i == 0x74 ||    // The End (Credits 1)
                    i == 0x75 ||    // Opening (Loop)
                    i == 0x76 ||    // Title Theme
                    i == 0x7B ||    // To the Moon
                    i == 0x7C ||    // The Giants' Exit
                    i == 0x7E ||    // Moon's Destruction
                    i == 0x7F       // Credits 2
                    ))
                    {
                        alreadyRolled[newSeqId] = true;
                    }
                    break;
                }
                else
                {
                    log_debug("Already rolled %02x for song %i. Rerolling...\n", newSeqId, i);
                }
            }
        }
        else
        {
            continue;
        }
        if(rc != VEC_SUCCESS)
        {
            log_error("Error with vector pop (index %i) ", i);
            vec_errmsg(rc);
        }
        else
        {
            AudioApi_ReplaceSequence(i, &(sequenceTableImpostor->entries[newSeqId]));
            char oldSeqName[256];
            char newSeqName[256];

            int offsetSeqId = newSeqId;
    
            // if (offsetSeqId >= 256)
            // {
            //     offsetSeqId -= (((newSeqId - 256)/254) * 2);
            //     offsetSeqId -= 128;
            // }
    
            vec_at(songNames, i, oldSeqName);

            if (offsetSeqId < 256)
            {
                vec_at(songNames, offsetSeqId, &(newSeqName[0]));
                randomizedIds[i] = offsetSeqId;
            }
            else
            {
                vec_at(songNames, offsetSeqId, &(newSeqName[0]));
                randomizedIds[i] = offsetSeqId;
            }

            log_info("[MUSIC RANDOMIZER] Replaced sequence %s with sequence %s.", oldSeqName, newSeqName)
            log_debug("(%i --> %i)", i, newSeqId);
            log_info("\n");
            
            AudioApi_ReplaceSequenceFont(i, 0, sequenceFontTableImpostor[((u16*)sequenceFontTableImpostor)[newSeqId] + 1]);
            // log_debug("Sequence font is now %02x\n", AudioApi_GetSequenceFont(newSeqId, 0));
        }

        vec_teardown(availableSeqs);
    }

    log_debug("Randomization finished! %i\n", sequenceTableImpostor->header.numEntries)
    // print_bytes(gAudioCtx.sequenceFontTable, (sizeof(u16) * gAudioCtx.sequenceTable->header.numEntries) + gAudioCtx.sequenceTable->header.numEntries * 20);

    // Cleanup
    for (int i = 0; i < 139; i++)
    {
        vec_teardown(categorySequences[i]);
    }
    recomp_free(categorySequences);

    for (int i = 0; i < gAudioCtx.sequenceTable->header.numEntries; i++)
    {
        vec_teardown(sequenceCategories[i]);
    }
    recomp_free(sequenceCategories);
    
    recomp_free(sequenceFontTableImpostor);    
}