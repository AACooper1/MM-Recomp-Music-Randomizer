#ifndef SONGSLOT_H
#define SONGSLOT_H

#include "seed.h"
#include "category.h"

class SongSlot
{
    public:
        SongSlot(int id, std::string name, const std::vector<int>& categories) : 
            name(name), categories(categories), id(id) {this->categories.push_back(id + 0x100);}
        
        bool is_fanfare() 
        {
            for (int i = 0; i < categories.size(); i++)
            {
                if (categories[i] == FANFARE || categories[i] == AREA_CLEAR || categories[i] == GAME_OVER)
                {
                    return true;
                }
            }
            return false;
        }
        
        std::string name;
        std::vector<int> categories;

        std::vector<int> availableTracks;
        std::vector<int> availableTracksNoRemove;
        int id = 0;
        std::shared_ptr<Track> vanillaTrack;
    private:
        std::shared_ptr<Seed> seed;
};

enum class SongSlotID
{
    TERMINA_FIELD = 0x02,
    CHASE,
    MAJORAS_THEME,
    CLOCK_TOWER,
    STONE_TOWER_TEMPLE,
    ELPMET_REWOT_ENOTS,
    MISSED_EVENT_1,
    MISSED_EVENT_2,
    HAPPY_MASK_SALESMAN,
    SONG_OF_HEALING,
    SOUTHERN_SWAMP,
    GHOST_ATTACK,
    SWAMP_CRUISE,
    SHARPS_CURSE,
    GREAT_BAY_COAST,
    IKANA_VALLEY,
    DEKU_PALACE,
    MOUNTAIN_VILLAGE,
    PIRATES_FORTRESS,
    CLOCK_TOWN_DAY_1,
    CLOCK_TOWN_DAY_2,
    CLOCK_TOWN_DAY_3,
    FILE_SELECT,
    EVENT_CLEAR,
    BATTLE,
    BOSS_BATTLE,
    WOODFALL_TEMPLE,
    SUNS_SONG,
    OPENING,
    HOUSE,
    GAME_OVER,
    BOSS_CLEAR,
    ITEM_CATCH,

    PTR_CLOCK_TOWN_DAY_2,
    
    GET_HEART_CONTAINER,
    MINIGAME,
    GORON_RACE,
    MUSIC_BOX_HOUSE,
    FAIRY_FOUNTAIN,
    ZELDAS_THEME,
    ROSA_SISTERS,
    OPEN_TREASURE_BOX,
    MARINE_RESEARCH_LABORATORY,
    GIANTS_THEME,
    GURU_GURUS_SONG,
    ROMANI_RANCH,
    GORON_VILLAGE,
    MAYORS_MEETING,

    OC_EPONAS_SONG,
    OC_SUNS_SONG,
    OC_SONG_OF_TIME,
    OC_SONG_OF_STORMS,

    ZORA_HALL,
    GET_MASK,
    MINIBOSS_BATTLE,
    SMALL_ITEM_CATCH,
    ASTRAL_OBSERVATORY,
    CAVERN,
    MILK_BAR,
    ZELDA_APPEARS,
    WOODS_OF_MYSTERY,
    GORON_RACE_GOAL,
    HORSE_RACE,
    HORSE_RACE_GOAL,
    GORMAN_TRACK,
    WITCHES_POTION_SHOP,
    SHOP,
    OWL,
    SHOOTING_GALLERY,

    OC_SONG_OF_SOARING,
    OC_SONG_OF_HEALING,
    OC_INVERTED_SONG_OF_TIME,
    OC_SONG_OF_DOUBLE_TIME,

    SONATA_OF_AWAKENING,
    GORON_LULLABY,
    NEW_WAVE_BOSSA_NOVA,
    ELEGY_OF_EMPTINESS,
    OATH_TO_ORDER,

    SWORDSMANS_SCHOOL,

    OC_GORON_LULLABY_INTRO,

    GET_OCARINA,
    BREMEN_MARCH,
    BALLAD_OF_THE_WIND_FISH,
    SONG_OF_SOARING,

    PTR_MILK_BAR,

    LAST_DAY,
    MIKAU,
    MIKAU_END,
    FROG_SONG,

    OC_SONATA_OF_AWAKENING,
    OC_GORON_LULLABY,
    OC_NEW_WAVE_BOSSA_NOVA,
    OC_ELEGY_OF_EMPTINESS,
    OC_OATH_TO_ORDER,

    PTR_LAST_DAY,
    PTR_OC_GORON_LULLABY_INTRO,

    BASS_GUITAR_SESSION,
    PIANO_SOLO,
    INDIGO_GOS,
    SNOWHEAD_TEMPLE,
    GREAT_BAY_TEMPLE,
    BOSSA_NOVA_SAX,
    BOSSA_NOVA_VOCALS,
    MAJORAS_WRATH,
    MAJORAS_INCARNATION,
    MAJORAS_MASK_BATTLE,
    BASS_PRACTICE,
    DRUMS_PRACTICE,
    PIANO_PRACTICE,
    IKANA_CASTLE,
    CALLING_GIANTS,
    KAMAROS_DANCE,
    CREMIAS_CARRIAGE,
    KEATONS_QUIZ,
    CREDITS_1,
    OPENING_LOOP,
    TITLE_THEME,
    WOODFALL_RISES,
    SOUTHERN_SWAMP_CLEARS,
    SNOWHEAD_CLEAR,

    UNK_00,

    TO_THE_MOON,
    THE_GIANTS_EXIT,
    TATL_TAEL,
    MOONS_DESTRUCTION,
    CREDITS_2
};

#endif