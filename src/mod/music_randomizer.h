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

const char* vanillaSongNames[256] = 
{
    /* 0x00 */ "General Sound Effects (should not roll)",
    /* 0x01 */ "Ambience (should not roll)",
    /* 0x02 */ "Termina Field",
    /* 0x03 */ "Chase",
    /* 0x04 */ "Majora's Theme",
    /* 0x05 */ "Clock Tower",
    /* 0x06 */ "Stone Tower Temple",
    /* 0x07 */ "Stone Tower Temple Upside-down",
    /* 0x08 */ "Missed Event 1",
    /* 0x09 */ "Missed Event 2",
    /* 0x0A */ "Happy Mask Salesman's Theme",
    /* 0x0B */ "Song of Healing",
    /* 0x0C */ "Southern Swamp",
    /* 0x0D */ "Ghost Attack",
    /* 0x0E */ "Swamp Cruise",
    /* 0x0F */ "Sharp's Curse",
    /* 0x10 */ "Great Bay Coast",
    /* 0x11 */ "Ikana Valley",
    /* 0x12 */ "Deku Palace",
    /* 0x13 */ "Mountain Village",
    /* 0x14 */ "Pirates' Fortress",
    /* 0x15 */ "Clock Town (Day 1)",
    /* 0x16 */ "Clock Town (Day 2)",
    /* 0x17 */ "Clock Town (Day 3)",
    /* 0x18 */ "File Select (Fairy Fountain)",
    /* 0x19 */ "Event Clear",
    /* 0x1A */ "Battle",
    /* 0x1B */ "Boss Battle",
    /* 0x1C */ "Woodfall Temple",
    /* 0x1D */ "Sun's Song (Morning Sequence)",
    /* 0x1E */ "Opening",
    /* 0x2F */ "House",
    /* 0x20 */ "Game Over",
    /* 0x21 */ "Boss Clear",
    /* 0x22 */ "Item Catch",
    /* 0x23 */ "A Pointer to Clock Town (Day 2) For Some Reason",
    /* 0x24 */ "Get a Heart Container!",
    /* 0x25 */ "Mini Game",
    /* 0x26 */ "Goron Race",
    /* 0x27 */ "Music Box House",
    /* 0x28 */ "Fairy's Fountain (File Select)",
    /* 0x29 */ "Zelda's Theme",
    /* 0x2A */ "Rosa Sisters",
    /* 0x2B */ "Open Treasure Box",
    /* 0x2C */ "Marine Research Laboratory",
    /* 0x2D */ "Giants' Theme",
    /* 0x2E */ "Guru-Guru's Song",
    /* 0x2F */ "Romani Ranch",
    /* 0x30 */ "Goron Village",
    /* 0x31 */ "Mayor's Meeting",
    /* 0x32 */ "", // Ocarina - Epona's Song
    /* 0x33 */ "", // Ocarina - Sun's Song
    /* 0x34 */ "", // Ocarina - Song of Time
    /* 0x35 */ "", // Ocarina - Song of Storms
    /* 0x36 */ "Zora Hall",
    /* 0x37 */ "Get a Mask!",
    /* 0x38 */ "Middle Boss Battle",
    /* 0x39 */ "Small Item Catch",
    /* 0x3A */ "Astral Observatory",
    /* 0x3B */ "Cavern",
    /* 0x3C */ "Milk Bar",
    /* 0x3D */ "Kafei Reveal",
    /* 0x3E */ "Woods of Mystery",
    /* 0x3F */ "Goron Race Goal",
    /* 0x40 */ "Horse Race",
    /* 0x41 */ "Horse Race Goal",
    /* 0x42 */ "Ingo",
    /* 0x43 */ "Witches' Potion Shop",
    /* 0x44 */ "Shop",
    /* 0x45 */ "Owl",
    /* 0x46 */ "Shooting Gallery",
    /* 0x47 */ "", // Ocarina - Song of Soaring
    /* 0x48 */ "", // Ocarina - Song of Healing
    /* 0x49 */ "", // Ocarina - Inverted Song of Time
    /* 0x4A */ "", // Ocarina - Song of Double Time
    /* 0x4B */ "Sonata of Awakening",
    /* 0x4C */ "Goron Lullaby",
    /* 0x4D */ "New Wave Bossa Nova",
    /* 0x4E */ "Elegy of Emptiness",
    /* 0x4F */ "Oath to Order",
    /* 0x50 */ "Swordsman's School",
    /* 0x51 */ "", // Ocarina - Goron Lullaby Intro,
    /* 0x52 */ "Get the Ocarina!", 
    /* 0x53 */ "Bremen March",
    /* 0x54 */ "Ballad of the Wind Fish",
    /* 0x55 */ "Song of Soaring",
    /* 0x56 */ "A Pointer to Milk Bar For Some Reason",
    /* 0x57 */ "Last Day",
    /* 0x58 */ "Mikau",
    /* 0x59 */ "Concert For You",
    /* 0x5A */ "Frog Song",
    /* 0x5B */ "", // Ocarina - Sonata of Awakening
    /* 0x5C */ "", // Ocarina - Goron Lullaby
    /* 0x5D */ "", // Ocarina - New Wave Bossa Nova
    /* 0x5E */ "", // Ocarina - Elegy of Emptiness
    /* 0x5F */ "", // Ocarina - Oath to Order
    /* 0x60 */ "Final Hours",
    /* 0x61 */ "A Pointer to Ocarina - Goron Lullaby Intro For Some Reason",
    /* 0x62 */ "Bass & Guitar Session",
    /* 0x63 */ "Piano Solo",
    /* 0x64 */ "The Indigo-Go's",
    /* 0x65 */ "Snowhead Temple",
    /* 0x66 */ "Great Bay Temple",
    /* 0x67 */ "New Wave Bossa Nova (Saxophone)",
    /* 0x68 */ "New Wave Bossa Nova (Vocals)",
    /* 0x69 */ "Majora's Wrath Battle",
    /* 0x6A */ "Majora's Incarnation Battle",
    /* 0x6B */ "Majora's Mask Battle",
    /* 0x6C */ "Bass Practice",
    /* 0x6D */ "Drums Practice",
    /* 0x6E */ "Piano Practice",
    /* 0x6F */ "Ikana Castle",
    /* 0x70 */ "Calling the Four Giants",
    /* 0x71 */ "Kamaro's Dance",
    /* 0x72 */ "Cremia's Carriage",
    /* 0x73 */ "Keaton's Quiz",
    /* 0x74 */ "The End (Credits 1)",
    /* 0x75 */ "Opening (Loop)",
    /* 0x76 */ "Title Theme",
    /* 0x77 */ "Woodfall Rises",
    /* 0x78 */ "Southern Swamp Clears",
    /* 0x79 */ "Snowhead Clear",
    /* 0x7A */ "", // Empty?
    /* 0x7B */ "To the Moon",
    /* 0x7C */ "The Giants' Exit",
    /* 0x7D */ "Tatl & Tael",
    /* 0x7E */ "Moon's Destruction",
    /* 0x7F */ "Credits 2"
};

Vector* songNames;
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