#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompui.h"

#include "mod_util.h"

#include "read_mmrs.h"

// Variables


// Enums
typedef enum Categories_t 
{
    CAT_FIELD,
    CAT_TOWN,
    CAT_DUNGEON,
    CAT_INDOORS,
    CAT_MINIGAME,
    CAT_ACTION,
    CAT_CALM,
    CAT_BOSS,
    CAT_FARFARE,
    CAT_GAME_OVER,
    CAT_AREA_CLEAR,
    CAT_SPECIAL = 16
} Categories;

Vector** categorySequences;
Vector** sequenceCategories;

AudioTable sequenceTableImpostor;
u8* sequenceFontTableImpostor;

void init_vanilla_sequence_categories();
void add_custom_sequence_categories(MMRS* allMmrs, int numMmrs);

Vector** init_catSeq_table()
{
    sequenceCategories = recomp_alloc(sizeof(Vector*) * gAudioCtx.sequenceTable->header.numEntries);
    for(int i = 0; i < gAudioCtx.sequenceTable->header.numEntries; i++)
    {
        sequenceCategories[i] = vec_init(sizeof(int));
    }
    // Categories for vanilla sequences. Pulled from MMR
    Vector* catSeqTemp[] = 
    {
        /*CAT_FIELD*/ 
        CAT_SEQ_INIT(
            (int[]) {
                NA_BGM_TERMINA_FIELD, 
                NA_BGM_MOUNTAIN_REGION, 
                NA_BGM_GREAT_BAY_REGION, 
                NA_BGM_IKANA_REGION,
                NA_BGM_SWAMP_REGION,
                NA_BGM_DEKU_PALACE,
                NA_BGM_ROMANI_RANCH,
                NA_BGM_SARIAS_SONG
            }, 8, sequenceCategories, 0
        ),

        /* CAT_TOWN */
        CAT_SEQ_INIT(
            (int[]) {
                NA_BGM_GREAT_BAY_REGION,
                NA_BGM_IKANA_REGION,
                NA_BGM_SWAMP_REGION,
                NA_BGM_CLOCK_TOWN_DAY_1,
                NA_BGM_CLOCK_TOWN_DAY_2,
                NA_BGM_CLOCK_TOWN_DAY_3,
                NA_BGM_GORON_VILLAGE,
                NA_BGM_ROMANI_RANCH,
                NA_BGM_ZORA_HALL,
                NA_BGM_DEKU_PALACE,
                NA_BGM_FAIRY_FOUNTAIN,
                NA_BGM_INGO,
                NA_BGM_SARIAS_SONG,
                NA_BGM_ZELDAS_LULLABY,
            }, 14, sequenceCategories, 1
        ),
        
        /* CAT_DUNGEON */
        CAT_SEQ_INIT(
            (int[]) {
                NA_BGM_CAVERN,
                NA_BGM_SNOWHEAD_TEMPLE,
                NA_BGM_GREAT_BAY_TEMPLE,
                NA_BGM_PIRATES_FORTRESS,
                NA_BGM_IKANA_CASTLE,
                NA_BGM_STONE_TOWER_TEMPLE,
                NA_BGM_INV_STONE_TOWER_TEMPLE,
                NA_BGM_WOODFALL_TEMPLE
            }, 8, sequenceCategories, 2
        ),

        /* CAT_INDOORS */
        CAT_SEQ_INIT(
            (int[]) {
                NA_BGM_GORON_VILLAGE,
                NA_BGM_ZORA_HALL,
                NA_BGM_CLOCK_TOWER,
                NA_BGM_SONG_OF_STORMS,
                NA_BGM_MILK_BAR,
                NA_BGM_INSIDE_A_HOUSE,
                NA_BGM_SHOP,
                NA_BGM_SHOOTING_GALLERY,
                NA_BGM_MARINE_RESEARCH_LAB,
                NA_BGM_ASTRAL_OBSERVATORY,
                NA_BGM_MUSIC_BOX_HOUSE,
                NA_BGM_CREMIA_CARRIAGE,
                NA_BGM_SWAMP_CRUISE,
                NA_BGM_MAYORS_OFFICE,
                NA_BGM_SWORD_TRAINING_HALL,
                NA_BGM_SHARPS_CURSE,
                NA_BGM_FAIRY_FOUNTAIN,
                NA_BGM_FILE_SELECT,
                NA_BGM_KOTAKE_POTION_SHOP,
                NA_BGM_SARIAS_SONG,
                NA_BGM_ZELDAS_LULLABY,
                NA_BGM_SONG_OF_HEALING,
                NA_BGM_GIANTS_THEME
            }, 23, sequenceCategories, 3
        ),

            /* CAT_MINIGAME */
        CAT_SEQ_INIT(
            (int[])
            {
                NA_BGM_MILK_BAR,
                NA_BGM_SHOOTING_GALLERY,
                NA_BGM_MUSIC_BOX_HOUSE,
                NA_BGM_GORON_RACE,
                NA_BGM_TIMED_MINI_GAME,
                NA_BGM_CREMIA_CARRIAGE,
                NA_BGM_SWAMP_CRUISE,
                NA_BGM_HORSE,
                NA_BGM_SWORD_TRAINING_HALL,
                NA_BGM_SARIAS_SONG,
                NA_BGM_MAJORAS_INCARNATION,
                NA_BGM_ENEMY
            }, 12, sequenceCategories, 4
        ),

        /* CAT_ACTION */
        CAT_SEQ_INIT(
            (int[]) {
                NA_BGM_TIMED_MINI_GAME,
                NA_BGM_MAYORS_OFFICE,
                NA_BGM_ALIEN_INVASION,
                NA_BGM_SWORD_TRAINING_HALL,
                NA_BGM_SHARPS_CURSE,
                NA_BGM_CHASE,
                NA_BGM_ZELDA_APPEAR,
                NA_BGM_MAJORAS_INCARNATION,
                NA_BGM_ENEMY
            }, 9, sequenceCategories, 5
        ),

        /* CAT_CALM */
        CAT_SEQ_INIT(
            (int[]) {
                NA_BGM_CLOCK_TOWER,
                NA_BGM_SONG_OF_STORMS,
                NA_BGM_MARINE_RESEARCH_LAB,
                NA_BGM_MUSIC_BOX_HOUSE,
                NA_BGM_CREMIA_CARRIAGE,
                NA_BGM_SWAMP_CRUISE,
                NA_BGM_MAYORS_OFFICE,
                NA_BGM_ALIEN_INVASION,
                NA_BGM_SHARPS_CURSE,
                NA_BGM_MAJORAS_THEME,
                NA_BGM_FAIRY_FOUNTAIN,
                NA_BGM_FILE_SELECT,
                NA_BGM_KOTAKE_POTION_SHOP,
                NA_BGM_INGO,
                NA_BGM_SARIAS_SONG,
                NA_BGM_ZELDAS_LULLABY,
                NA_BGM_TATL_AND_TAEL,
                NA_BGM_SONG_OF_HEALING,
                NA_BGM_GIANTS_THEME,
                NA_BGM_GATHERING_GIANTS,
                NA_BGM_TITLE_THEME
            }, 21, sequenceCategories, 6
        ),

        /* CAT_BOSS */
        CAT_SEQ_INIT(
            (int[])
            {
                NA_BGM_CHASE,
                NA_BGM_MINI_BOSS,
                NA_BGM_BOSS,
                NA_BGM_MAJORAS_MASK,
                NA_BGM_MAJORAS_INCARNATION,
                NA_BGM_MAJORAS_WRATH
            }, 6, sequenceCategories, 7
        ),

        /* CAT_FANFARE */
        CAT_SEQ_INIT(
            (int[])
            {
                NA_BGM_CLEAR_EVENT,
                NA_BGM_GET_ITEM,
                NA_BGM_GET_HEART,
                NA_BGM_GET_NEW_MASK,
                NA_BGM_GET_SMALL_ITEM,
                NA_BGM_ZELDA_APPEAR,
                NA_BGM_GORON_GOAL,
                NA_BGM_HORSE_GOAL,
                NA_BGM_LEARNED_NEW_SONG,
                NA_BGM_SONG_OF_SOARING,
                NA_BGM_DUNGEON_APPEAR
            }, 11, sequenceCategories, 8
        ),

        /* CAT_GAME_OVER */
        CAT_SEQ_INIT(
            (int[])
            {
                NA_BGM_FAILURE_0,
                NA_BGM_FAILURE_1,
                NA_BGM_GAME_OVER,
                NA_BGM_CLEAR_BOSS,
                NA_BGM_SONG_OF_SOARING,
                NA_BGM_DUNGEON_APPEAR,
            }, 6, sequenceCategories, 9
        ),

            // CAT_AREA_CLEAR
        CAT_SEQ_INIT(
            (int[])
            {
                NA_BGM_CLEAR_BOSS,
                NA_BGM_WOODFALL_CLEAR,
                NA_BGM_SNOWHEAD_CLEAR,
                NA_BGM_MOONS_DESTRUCTION,
                NA_BGM_GOODBYE_GIANT,
            }, 5, sequenceCategories, 10
        ),
        
        /* CAT_SPECIAL */
        CAT_SEQ_INIT(
            (int[])
            {
                NA_BGM_GATHERING_GIANTS,
                NA_BGM_TITLE_THEME,
                NA_BGM_FINAL_HOURS
            }, 3, sequenceCategories, 11
        )
    };

    Vector** catSeqPerm = recomp_alloc(sizeof(Vector*) * (12 + 127));
    Lib_MemCpy(catSeqPerm, catSeqTemp, sizeof(Vector*) * 12);

    // REMINDER! When adding logic for individual song slots, remember to subtract 0xF4 if category[i] > 16.
    // And also if category[i] == 16 add it to category 11.
    for (int i = 12; i < 139; i++)
    {
        log_debug("\nCreating entry for song %i\n", i);
        Vector* songSlot = CAT_SEQ_INIT((int[]){i}, 1, sequenceCategories, i);
        log_debug("Created!\n");
        log_debug("Data is %i\n", *((int*)songSlot->dataStart));
        log_debug("%p, %p\n", &(catSeqPerm[i]), &songSlot);
        Lib_MemCpy(&(catSeqPerm[i]), &songSlot, sizeof(Vector*));
        log_debug("Copied to address %p!\n", &(catSeqPerm[i]));
    }

    return catSeqPerm;
}