#include "music_randomizer.h"

RECOMP_IMPORT(".", int set_log_level(int level));
RECOMP_IMPORT("magemods_audio_api", void AudioApi_ReplaceSequence(s32 seqId, AudioTableEntry* entry));
RECOMP_IMPORT("magemods_audio_api", void AudioApi_ReplaceSequenceFont(s32 seqId, s32 fontNum, s32 fontId));

RECOMP_DECLARE_EVENT(music_rando_on_init());

// Initializes the categories table and populates them with vanilla sequences.
void init_vanilla_sequence_categories()
{
    categorySequences = init_catSeq_table();
    log_debug("Initialized Category-->Sequences table at address %p.\n", categorySequences);

    for (size_t i = 0; i < 128; i++)
    {
        vec_push_back(songNames, vanillaSongNames[i]);
        char* thisSongName = (char*)recomp_alloc(sizeof(char) * 256);
        Lib_MemSet(thisSongName, 0, 256);
        vec_at(songNames, i, thisSongName);
    }
}

// Add all custom sequences to the categories table.
void add_custom_sequence_categories(MMRS* allMmrs, int numMmrs)
{
    for (int i = 0; i < numMmrs; i++)
    {
        // Custom music tracks start at index 256 in Mage's API. Subject to change.
        int seqId = i + 256;
        // Offset to account for 0xXFE and 0xXFF being reserved
        seqId += ((seqId - 256)/254) * 2;

        vec_push_back(songNames, allMmrs[i].songName);
        for (int c = 0; c < 256; c++)
        {
            if (allMmrs[i].categories[c])
            {
                // Canonical "Category"
                if (c < 0x16)
                {
                    vec_push_back(categorySequences[c], &seqId);
                }
                // Final Hours/Title Screen
                else if (c == 0x16)
                {
                    int c_offset = 11;
                    vec_push_back(categorySequences[c_offset], &seqId);
                }
                // Should not exist
                else if (c < 0x102)
                {
                    log_error("Error: Song %i has category %d, which does not exist.\n", i, c);
                    continue;
                }
                // Individual song slot
                else
                {
                    int c_offset = c - 0xF5;
                    vec_push_back(categorySequences[c_offset], &seqId);
                }
            }
        }
    }
}

RECOMP_CALLBACK(".", mmrs_reader_done) void init_music_rando(MMRS* allMmrs, int numMmrs)
{
    songNames = vec_init(256);
    init_vanilla_sequence_categories();

    // Rand_Seed(get_current_time());

    add_custom_sequence_categories(allMmrs, numMmrs);

    sequenceFontTableImpostor = recomp_alloc((sizeof(u16) * gAudioCtx.sequenceTable->header.numEntries) + gAudioCtx.sequenceTable->header.numEntries * 20);
    Lib_MemCpy(sequenceFontTableImpostor, gAudioCtx.sequenceFontTable, (sizeof(u16) * gAudioCtx.sequenceTable->header.numEntries) + gAudioCtx.sequenceTable->header.numEntries * 20);

    // print_bytes(gAudioCtx.sequenceFontTable, gAudioCtx.sequenceTable->header.numEntries * 5);

    log_debug("Copying AudioTable... ")
    
    // Header
    AudioTableHeader copyHeader = {
        /*   numEntries   */ gAudioCtx.sequenceTable->header.numEntries,
        /* unkMediumParam */ 0,
        /*    romAddr     */ (uintptr_t)recomp_alloc(sizeof(AudioTableEntry) * gAudioCtx.sequenceTable->header.numEntries),
        /*      pad       */ {0, 0, 0, 0, 0, 0, 0, 0}
    };
    sequenceTableImpostor.header = copyHeader;

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

        sequenceTableImpostor.entries[i] = copyEntry;
    }

    if (logLevel >= LOG_DEBUG)
    {
        log_debug("Copied!\n");
        print_bytes(&(sequenceTableImpostor.entries[2]), sizeof(AudioTableEntry));
    }

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

    bool alreadyRolled[gAudioCtx.sequenceTable->header.numEntries];
    for (int i = 0; i < gAudioCtx.sequenceTable->header.numEntries; i++)
    {
        alreadyRolled[i] = false;
    }

    randomizedIds = recomp_alloc(sizeof(int) * gAudioCtx.sequenceTable->header.numEntries);
    for (int z = 0; z < gAudioCtx.sequenceTable->header.numEntries; z++)
    {
        randomizedIds[z] = z;
    }

    for (int i = 2; i < sequenceTableImpostor.header.numEntries; i++)
    {
        Vector* availableSeqs = vec_init(sizeof(int));

        // Get the pool of sequences to pull from
        AudioTableEntry* thisEntry = &(sequenceTableImpostor.entries[i]);
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
                    log_debug("Rolled 0x%02x for song %i, which is a pointer. Rerolling...", newSeqId, i)
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
            AudioApi_ReplaceSequence(i, &(sequenceTableImpostor.entries[newSeqId]));
            char oldSeqName[256];
            char newSeqName[256];
            vec_at(songNames, i, oldSeqName);

            if (newSeqId < 256)
            {
                vec_at(songNames, newSeqId, &(newSeqName[0]));
                randomizedIds[i] = newSeqId;
            }
            else
            {
                vec_at(songNames, newSeqId - 128, &(newSeqName[0]));
                randomizedIds[i] = newSeqId - 128;
            }

            log_debug("[MUSIC RANDOMIZER] Replaced sequence %i with sequence %i.\n", i, newSeqId);
            log_info("[MUSIC RANDOMIZER] Replaced sequence %s with sequence %s.\n", oldSeqName, newSeqName)
            AudioApi_ReplaceSequenceFont(i, 0, sequenceFontTableImpostor[((u16*)sequenceFontTableImpostor)[newSeqId] + 1]);
        }

    }

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