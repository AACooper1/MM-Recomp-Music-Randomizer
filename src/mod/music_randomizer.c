#include "music_randomizer.h"

RECOMP_IMPORT(".", int set_log_level(int level));
RECOMP_IMPORT("magemods_audio_api", void AudioApi_ReplaceSequence(s32 seqId, AudioTableEntry* entry));
RECOMP_IMPORT("magemods_audio_api", void AudioApi_ReplaceSequenceFont(s32 seqId, s32 fontNum, s32 fontId));

RECOMP_DECLARE_EVENT(music_rando_on_init());

// Initializes the categories table and populates them with vanilla sequences.
void init_vanilla_sequence_categories()
{
    log_debug("Initializing category sequence table. Probably about to crash. \n");
    categorySequences = init_catSeq_table();
    log_debug("CatSeq table at address %p.\n", categorySequences);
    // print_bytes(categorySequences, sizeof(Vector*));

    // print_bytes(categorySequences[0], sizeof(Vector));
    for (size_t i = 0; i < 127; i++)
    {
        log_debug("%s (index %i) at %p\n", vanillaSongNames[i], i, &(vanillaSongNames[i]))
        log_debug("songNames vector at %p.\n", songNames);
        vec_push_back(songNames, vanillaSongNames[i]);
        log_debug("Pushed.\n")
        char* thisSongName = (char*)recomp_alloc(sizeof(char) * 256);
        Lib_MemSet(thisSongName, 0, 256);
        vec_at(songNames, i, thisSongName);
    }
}

// Add all custom sequences to the categories table.
void add_custom_sequence_categories(MMRS* allMmrs, int numMmrs)
{
    log_debug("\nCALLED add_custom_sequence_categories()\n");

    // STOP_EXECUTION("\n...\n");

    for (int i = 0; i < numMmrs; i++)
    {
        // Custom music tracks start at index 256 in Mage's API. Subject to change.
        int seqId = i + 256;
        log_debug("Song name is %s.\n", allMmrs[i].songName);
        vec_push_back(songNames, allMmrs[i].songName);
        for (int c = 0; c < 256; c++)
        {
            if (allMmrs[i].categories[c])
            {
                log_debug("Song %s has category %i!\n", allMmrs[i].songName, c)
                // Canonical "Category"
                if (c < 0x16)
                {
                    log_debug("Pushing %i to %p...\n", seqId, categorySequences[i+12]);
                    vec_push_back(categorySequences[c], &seqId);
                }
                // Final Hours/Title Screen
                else if (c == 0x16)
                {
                    int c_offset = 11;
                    log_debug("Pushing %i to %p...\n", seqId, categorySequences[i+12]);
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
                    log_debug("Pushing %i to %p...\n", seqId, categorySequences[i+12]);
                    vec_push_back(categorySequences[c_offset], &seqId);
                }
                log_debug("Success!\n");
            }
        }
    }
}

RECOMP_CALLBACK(".", mmrs_reader_done) void init_music_rando(MMRS* allMmrs, int numMmrs)
{
    songNames = vec_init(256);
    init_vanilla_sequence_categories();

    add_custom_sequence_categories(allMmrs, numMmrs);
    
    for (int i = 0; i < 139; i++)
    {
        vec_randomize(categorySequences[i]);
    }

    sequenceFontTableImpostor = recomp_alloc((sizeof(u16) * gAudioCtx.sequenceTable->header.numEntries) + gAudioCtx.sequenceTable->header.numEntries * 20);
    Lib_MemCpy(sequenceFontTableImpostor, gAudioCtx.sequenceFontTable, (sizeof(u16) * gAudioCtx.sequenceTable->header.numEntries) + gAudioCtx.sequenceTable->header.numEntries * 20);

    // print_bytes(gAudioCtx.sequenceFontTable, gAudioCtx.sequenceTable->header.numEntries * 5);

    log_debug("Copying AudioTable...\n")
    
    // Header
    AudioTableHeader copyHeader = {
        /*   numEntries   */ gAudioCtx.sequenceTable->header.numEntries,
        /* unkMediumParam */ 0,
        /*    romAddr     */ (uintptr_t)recomp_alloc(sizeof(AudioTableEntry) * gAudioCtx.sequenceTable->header.numEntries),
        /*      pad       */ {0, 0, 0, 0, 0, 0, 0, 0}
    };
    sequenceTableImpostor.header = copyHeader;
    log_debug("\nCopy header:\n");
    


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
        log_debug("Copied! First (musical) entry:\n");
        print_bytes(&(sequenceTableImpostor.entries[2]), sizeof(AudioTableEntry));
    }

    music_rando_on_init();
}

RECOMP_CALLBACK(".", music_rando_on_init) void randomize_music()
{
    logLevel = set_log_level(LOG_INFO);
    bool alreadyRolled[gAudioCtx.sequenceTable->header.numEntries];
    for (int i = 0; i < gAudioCtx.sequenceTable->header.numEntries; i++)
    {
        alreadyRolled[i] = false;
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
                log_debug("\nPopped value is %02x.\n", newSeqId);
                // 15% chance of reroll if not custom song (possibly tweak later)
                if (newSeqId < 256)
                {
                    if (Rand_Next() % 100 < 15)
                    {
                        log_debug("Rerolling 0x%02x because custom music makers are GREEDY...\n", newSeqId);
                        continue;
                    }
                }
                // Reroll if already rolled and still has seqs available.
                if (alreadyRolled[newSeqId] == false)
                {
                    alreadyRolled[newSeqId] = true;
                    break;
                }
                else
                {
                    log_debug("Already rolled %02x. Rerolling...\n", newSeqId);
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
            }
            else
            {
                vec_at(songNames, newSeqId - 129, &(newSeqName[0]));
            }

            log_debug("[MUSIC RANDOMIZER] Replaced sequence %i with sequence %i.\n", i, newSeqId);
            log_info("[MUSIC RANDOMIZER] Replaced sequence %s with sequence %s.\n", oldSeqName, newSeqName)
            AudioApi_ReplaceSequenceFont(i, 0, sequenceFontTableImpostor[((u16*)sequenceFontTableImpostor)[newSeqId] + 1]);
            // log_info("Loaded audiobank %i.\n", sequenceFontTableImpostor[((u16*)sequenceFontTableImpostor)[newSeqId] + 1]);
        }

    }

    // print_bytes(sequenceFontTableImpostor, gAudioCtx.sequenceTable->header.numEntries * 5);

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
    // recomp_free(&sequenceTableImpostor);



    
}

// Cleanup. Remember to add this at the end of randomization algorithm.
// vec_teardown(categorySequences);