#include "ootrs.hpp"

OoTAudioBin::OoTAudioBin(fs::path path)
{
    this->audiobinPath = path;

    memset(&this->archive, 0, sizeof(mz_zip_archive));
    if (!mz_zip_reader_init_file(&archive, path.string().c_str(), 0))
    {
        logger.error << "Error reading zip file " << path.string() << ": " << mz_zip_get_error_string(mz_zip_get_last_error(&archive)) << std::endl;
    }
    else
    {
        logger.debug << "Reading " << this->audiobinPath << "..." << std::endl;

        int num_files = (int) mz_zip_reader_get_num_files(&archive);

        if (num_files < 4)
        {
            logger.error << "Error reading audiobin file: " << num_files << " files detected, but expected 4!" << std::endl;
        }
        else
        {
            for (int i = 0; i < num_files; i++)
            {
                mz_zip_archive_file_stat stat;

                if (!mz_zip_reader_file_stat(&archive, i, &stat))
                {
                    logger.error << "Error reading file " << stat.m_filename << ", skipping!" << std::endl;
                    continue;
                }
                
                int filesize = stat.m_uncomp_size;

                std::string filename = stat.m_filename;
                std::vector<char> filebuffer(filesize);

                if (!mz_zip_reader_extract_to_mem(&archive, i, filebuffer.data(), filebuffer.size(), 0))
                {
                    logger.critical << "While reading audiobin, mz_zip_reader_extract_to_mem() failed!" << std::endl;
                    throw std::runtime_error("mz_zip_reader_extract_to_mem() failed");
                    break;
                }
                raw_files.insert(std::make_pair(filename, std::vector<char>(filesize)));
                for (int j = 0; j < filesize; j++)
                {
                    raw_files[filename].data()[j] = filebuffer.data()[j ^ 3];
                }

            }
            for (int i = 0; i < 4; i++)
            {
                if (!raw_files.contains(expected_files[i]))
                {
                    logger.error << "Audiobin did not contain " << expected_files[i] << ", aborting!" << std::endl;                    
                    successfully_parsed = false;
                    return;
                }
                else
                {
                    logger.debug << "Found file " << expected_files[i] << "!" << std::endl;
                }
            }

            soundTableHeader = &raw_files[AUDIOTABLE_HEADER];
            soundTable = &raw_files[AUDIOTABLE];
            bankTableHeader = &raw_files[BANKTABLE_HEADER];
            bankTable = &raw_files[BANKTABLE];
            
            successfully_parsed = true;
        }
    }

    mz_zip_reader_end(&archive);
}

std::unordered_map<std::string, std::vector<SongSlotID>> OoTBGMGroupsToCategories
{
    // Exact specificity
    {"HyruleField", {SongSlotID::TERMINA_FIELD}},
    {"LostWoods", {SongSlotID::WOODS_OF_MYSTERY}},
    {"GerudoValley", {SongSlotID::PIRATES_FORTRESS}},
    {"Market", {SongSlotID::MUSIC_BOX_HOUSE}}, // I like Kokiri as Clock Town Day 1 more. It's the first area, the hub
    {"KakarikoChild", {SongSlotID::CLOCK_TOWN_DAY_2}},
    {"KakarikoAdult", {SongSlotID::CLOCK_TOWN_DAY_3}},
    {"LonLonRanch", {SongSlotID::ROMANI_RANCH}},
    {"KokiriForest", {SongSlotID::CLOCK_TOWN_DAY_1}},
    {"GoronCity", {SongSlotID::GORON_VILLAGE}},
    {"ZorasDomain", {SongSlotID::ZORA_HALL}},
    {"CastleCourtyard", {SongSlotID::DEKU_PALACE}},
    {"HorseRace", {SongSlotID::HORSE_RACE}},
    {"Mini-game", {SongSlotID::MINIGAME}},
    {"ShootingGallery", {SongSlotID::SHOOTING_GALLERY}},
    {"FairyFountain", {SongSlotID::FAIRY_FOUNTAIN}},
    {"TempleOfTime", {SongSlotID::CLOCK_TOWER}}, // Was going to be Giants' Theme, but that fits Chamber of the Sages better
    {"ChamberOfTheSages", {SongSlotID::GIANTS_THEME}},
    {"House", {SongSlotID::HOUSE}},
    {"Shop", {SongSlotID::SHOP}},
    {"PotionShop", {SongSlotID::MARINE_RESEARCH_LABORATORY}}, // Koume and Kotake should have Koume and Kotake, of course
    {"WindmillHut", {SongSlotID::GURU_GURUS_SONG}},
    {"InsideDekuTree", {SongSlotID::CAVERN}},
    {"DodongosCavern", {SongSlotID::CAVERN}}, // Can't really think of anything better. All ambient sound is just CAVERN
    {"JabuJabu", {SongSlotID::GHOST_ATTACK}}, // Idk
    {"ForestTemple", {SongSlotID::WOODFALL_TEMPLE}},
    {"IceCavern", {SongSlotID::SNOWHEAD_TEMPLE}},
    {"WaterTemple", {SongSlotID::GREAT_BAY_TEMPLE}},
    {"ShadowTemple", {SongSlotID::STONE_TOWER_TEMPLE}},
    {"SpiritTemple", {SongSlotID::ELPMET_REWOT_ENOTS}},
    {"CastleUnderground", {SongSlotID::MAJORAS_MASK_BATTLE}}, // Ganon's Theme
    {"CastleEscape", {SongSlotID::LAST_DAY}}, // Would be good for To The Moon, but that is not heard in rando (unfortunately. banger)
    {"Battle", {SongSlotID::BATTLE}},
    {"MinibossBattle", {SongSlotID::MINIBOSS_BATTLE}},
    {"BossBattle", {SongSlotID::BOSS_BATTLE}},
    {"FireBoss", {SongSlotID::CHASE}},
    {"GanondorfBattle", {SongSlotID::MAJORAS_INCARNATION}},
    {"GanonBattle", {SongSlotID::MAJORAS_WRATH}},
    {"TitleTheme", {SongSlotID::TITLE_THEME}},
    {"ZeldaTheme", {SongSlotID::ZELDAS_THEME}},
    {"SheikTheme", {SongSlotID::ASTRAL_OBSERVATORY}}, // I feel like there's not a huge parallel. HMS maybe but you never hear him in rando. They both sound nice and peaceful though I guess
    {"DekuTree", {SongSlotID::SHARPS_CURSE}},
    {"KaeporaGaebora", {SongSlotID::OWL}},
    {"FairyFlying", {SongSlotID::SWAMP_CRUISE}},
    {"KotakeAndKoume", {SongSlotID::WITCHES_POTION_SHOP}},
    {"IngoTheme", {SongSlotID::GORMAN_TRACK}},

    // High specificity
    {
        "Fields", // == MMR's {FIELD} + WoM
        {
            SongSlotID::TERMINA_FIELD, 
            SongSlotID::SOUTHERN_SWAMP, 
            SongSlotID::WOODS_OF_MYSTERY,
            SongSlotID::MOUNTAIN_VILLAGE,
            SongSlotID::GREAT_BAY_COAST,
            SongSlotID::IKANA_VALLEY
        }
    },
    {
        "Town", // == MMR's {TOWN}
        {
            SongSlotID::CLOCK_TOWN_DAY_1,
            SongSlotID::CLOCK_TOWN_DAY_2,
            SongSlotID::CLOCK_TOWN_DAY_3,
            SongSlotID::GORON_VILLAGE,
            SongSlotID::ROMANI_RANCH,
            SongSlotID::ZORA_HALL,
            SongSlotID::DEKU_PALACE
        }
    },
    {
        "Fun", // == MMR's {MINIGAME}
        {
            SongSlotID::HORSE_RACE,
            SongSlotID::GORON_RACE,
            SongSlotID::MINIGAME,
            SongSlotID::CREMIAS_CARRIAGE,
            SongSlotID::SWAMP_CRUISE
        }
    },
    {
        "MagicalPlace",
        {
            SongSlotID::FAIRY_FOUNTAIN,
            SongSlotID::MARINE_RESEARCH_LABORATORY,
            SongSlotID::ASTRAL_OBSERVATORY, // Of course
            SongSlotID::CLOCK_TOWER,
            SongSlotID::KEATONS_QUIZ,
            SongSlotID::GIANTS_THEME
        },
    },
    {
        "SalesArea",
        {
            SongSlotID::SHOP,
            SongSlotID::MILK_BAR,
            SongSlotID::WITCHES_POTION_SHOP
        }
    },
    {
        "ChildDungeon",
        {
            SongSlotID::WOODFALL_TEMPLE,
            SongSlotID::CAVERN
        }
    },
    {
        "AncientDungeon",
        {
            SongSlotID::STONE_TOWER_TEMPLE,
            SongSlotID::ELPMET_REWOT_ENOTS
        }
    },
    {
        "MysticalDungeon",
        {
            SongSlotID::SNOWHEAD_TEMPLE,
            SongSlotID::GREAT_BAY_TEMPLE
        }
    },
    {
        "SpookyDungeon",
        {
            SongSlotID::IKANA_CASTLE,
            SongSlotID::PIRATES_FORTRESS
        }
    },
    {
        "SmallFight",
        {
            SongSlotID::BATTLE,
            SongSlotID::CHASE
        }
    },
    {
        "BossFight",
        {
            SongSlotID::MINIBOSS_BATTLE,
            SongSlotID::BOSS_BATTLE,
        }
    },
    {
        "FinalFight",
        {
            SongSlotID::TO_THE_MOON,
            SongSlotID::MAJORAS_MASK_BATTLE,
            SongSlotID::MAJORAS_INCARNATION,
            SongSlotID::MAJORAS_WRATH
        }
    },
    {
        "HeroTheme",
        {
            SongSlotID::TITLE_THEME,
            SongSlotID::ZELDAS_THEME,
            SongSlotID::OWL,
            SongSlotID::SWORDSMANS_SCHOOL,
            SongSlotID::TATL_TAEL,
            SongSlotID::GIANTS_THEME
        }
    },
    {
        "VillainTheme",
        {
            SongSlotID::SHARPS_CURSE,
            SongSlotID::MAJORAS_THEME,
            SongSlotID::GORMAN_TRACK,
            SongSlotID::MAYORS_MEETING
        }
    },

    // Mid specificity
    {
        "Outdoors", // == MMR's {FIELD, TOWN, MINIGAME}
        {
            SongSlotID::TERMINA_FIELD,
            SongSlotID::SOUTHERN_SWAMP,
            SongSlotID::MOUNTAIN_VILLAGE,
            SongSlotID::GREAT_BAY_COAST,
            SongSlotID::IKANA_VALLEY,
            SongSlotID::CLOCK_TOWN_DAY_1,
            SongSlotID::CLOCK_TOWN_DAY_2,
            SongSlotID::CLOCK_TOWN_DAY_3,
            SongSlotID::ROMANI_RANCH,
            SongSlotID::GORMAN_TRACK,
            SongSlotID::GORON_VILLAGE, // Zora's Domain and Goron City are underground
            SongSlotID::ZORA_HALL, // Are they stupid??
            SongSlotID::DEKU_PALACE,
            SongSlotID::HORSE_RACE,
            SongSlotID::GORON_RACE,
            SongSlotID::MINIGAME,
            SongSlotID::CREMIAS_CARRIAGE,
            SongSlotID::SWAMP_CRUISE
        }
    },
    {
        "Indoors", // == MMR's {INDOORS} + some more
        {
            SongSlotID::CLOCK_TOWER,
            SongSlotID::GURU_GURUS_SONG,
            SongSlotID::HOUSE,
            SongSlotID::SHOP,
            SongSlotID::SHOOTING_GALLERY,
            SongSlotID::MARINE_RESEARCH_LABORATORY,
            SongSlotID::ASTRAL_OBSERVATORY,
            SongSlotID::MUSIC_BOX_HOUSE,
            SongSlotID::MAYORS_MEETING,
            SongSlotID::SWORDSMANS_SCHOOL,
            SongSlotID::FAIRY_FOUNTAIN,
            SongSlotID::WITCHES_POTION_SHOP,
        }
    },
    {
        "AdultDungeon", // Full dungeons
        {
            SongSlotID::WOODFALL_TEMPLE,
            SongSlotID::SNOWHEAD_TEMPLE,
            SongSlotID::GREAT_BAY_TEMPLE,
            SongSlotID::STONE_TOWER_TEMPLE,
            SongSlotID::ELPMET_REWOT_ENOTS
        }
    },
    {
        "BigFight", // == MMR's {BOSS}
        {
            SongSlotID::MINIBOSS_BATTLE,
            SongSlotID::BOSS_BATTLE,
            SongSlotID::MAJORAS_MASK_BATTLE,
            SongSlotID::MAJORAS_INCARNATION,
            SongSlotID::MAJORAS_WRATH
        }
    },

    // Low Specificity
    {
        "Overworld", // == MMR's {FIELD, TOWN, INDOORS, MINIGAME} + some cutscenes
        {
            SongSlotID::TERMINA_FIELD,
            SongSlotID::SOUTHERN_SWAMP,
            SongSlotID::MOUNTAIN_VILLAGE,
            SongSlotID::GREAT_BAY_COAST,
            SongSlotID::IKANA_VALLEY,
            SongSlotID::CLOCK_TOWN_DAY_1,
            SongSlotID::CLOCK_TOWN_DAY_2,
            SongSlotID::CLOCK_TOWN_DAY_3,
            SongSlotID::ROMANI_RANCH,
            SongSlotID::GORMAN_TRACK,
            SongSlotID::GORON_VILLAGE, 
            SongSlotID::ZORA_HALL,
            SongSlotID::DEKU_PALACE,
            SongSlotID::HORSE_RACE,
            SongSlotID::GORON_RACE,
            SongSlotID::MINIGAME,
            SongSlotID::CREMIAS_CARRIAGE,
            SongSlotID::SWAMP_CRUISE,
            SongSlotID::CLOCK_TOWER,
            SongSlotID::GURU_GURUS_SONG,
            SongSlotID::HOUSE,
            SongSlotID::SHOP,
            SongSlotID::SHOOTING_GALLERY,
            SongSlotID::MARINE_RESEARCH_LABORATORY,
            SongSlotID::ASTRAL_OBSERVATORY,
            SongSlotID::MUSIC_BOX_HOUSE,
            SongSlotID::MAYORS_MEETING,
            SongSlotID::SWORDSMANS_SCHOOL,
            SongSlotID::FAIRY_FOUNTAIN,
            SongSlotID::WITCHES_POTION_SHOP
        }
    },
    {
        "Dungeon", // == MMR's {DUNGEON}
        {
            SongSlotID::CAVERN,
            SongSlotID::WOODFALL_TEMPLE,
            SongSlotID::SNOWHEAD_TEMPLE,
            SongSlotID::GREAT_BAY_TEMPLE,
            SongSlotID::IKANA_CASTLE,
            SongSlotID::PIRATES_FORTRESS,
            SongSlotID::STONE_TOWER_TEMPLE,
            SongSlotID::ELPMET_REWOT_ENOTS
        }
    },
    {
        "Fight", // == MMR's {BOSS + ACTION}
        {
            SongSlotID::BATTLE,
            SongSlotID::MINIBOSS_BATTLE,
            SongSlotID::BOSS_BATTLE,
            SongSlotID::MAJORAS_MASK_BATTLE,
            SongSlotID::MAJORAS_INCARNATION,
            SongSlotID::MAJORAS_WRATH,
            SongSlotID::TO_THE_MOON,
            SongSlotID::CHASE,
            SongSlotID::MAYORS_MEETING,
            SongSlotID::SHARPS_CURSE
        }
    },
    {
        "CharacterTheme",
        {
            SongSlotID::TITLE_THEME,
            SongSlotID::ZELDAS_THEME,
            SongSlotID::OWL,
            SongSlotID::SWORDSMANS_SCHOOL,
            SongSlotID::TATL_TAEL,
            SongSlotID::GIANTS_THEME,
            SongSlotID::SHARPS_CURSE,
            SongSlotID::MAJORAS_THEME,
            SongSlotID::GORMAN_TRACK,
            SongSlotID::MAYORS_MEETING
        }
    }
};

std::unordered_map<std::string, std::vector<SongSlotID>> OoTFanfareGroupsToCategories
{
    // Ignore Ocarina songs, as they work differently in MM vs. Ocarina
    
    // Exact specificity
    {"ItemGet", {SongSlotID::ITEM_CATCH}},
    {"HeartContainerGet", {SongSlotID::GET_HEART_CONTAINER}},
    {"SpiritStoneGet", {SongSlotID::SOUTHERN_SWAMP_CLEARS}}, // Dungeon Clear, short
    {"HeartPieceGet", {SongSlotID::SMALL_ITEM_CATCH}},
    {"MedallionGet", {SongSlotID::SNOWHEAD_CLEAR}}, // Dungeon Clear, long
    {"LearnSong", {SongSlotID::GET_OCARINA}},
    {"BossDefeated", {SongSlotID::BOSS_CLEAR}},
    {"EponaRaceGoal", {SongSlotID::HORSE_RACE_GOAL}},
    {"EscapeFromRanch", {SongSlotID::EVENT_CLEAR}},
    {"TreasureChest", {SongSlotID::OPEN_TREASURE_BOX}},
    {"MasterSword", {SongSlotID::MOONS_DESTRUCTION}},
    {"DoorOfTime", {SongSlotID::WOODFALL_RISES}},
    {"ZeldaTurnsAround", {SongSlotID::ZELDA_APPEARS}},
    {"GanondorfAppears", {SongSlotID::MISSED_EVENT_1}}, // idfk
    {"GameOver", {SongSlotID::GAME_OVER}},

    // Mid specificity
    {
        "ItemFanfare", 
        {
            SongSlotID::ITEM_CATCH,
            SongSlotID::SMALL_ITEM_CATCH,
            SongSlotID::GET_HEART_CONTAINER,
            SongSlotID::GET_MASK,
            SongSlotID::GET_OCARINA
        }
    },
    {
        "SuccessFanfare",
        {
            SongSlotID::BOSS_CLEAR,
            SongSlotID::SNOWHEAD_CLEAR,
            SongSlotID::SOUTHERN_SWAMP_CLEARS,
            SongSlotID::EVENT_CLEAR,
            SongSlotID::GORON_RACE_GOAL,
            SongSlotID::HORSE_RACE_GOAL,
            SongSlotID::WOODFALL_RISES
        }
    },
    {
        "BigFanfare",
        {
            SongSlotID::BOSS_CLEAR,
            SongSlotID::OPEN_TREASURE_BOX,
            SongSlotID::SNOWHEAD_CLEAR,
            SongSlotID::SOUTHERN_SWAMP_CLEARS,
            SongSlotID::MOONS_DESTRUCTION,
            SongSlotID::THE_GIANTS_EXIT,
            SongSlotID::WOODFALL_RISES
        }
    },

    // Low specificity
    {
        "EventFanfare", // == MMR's {FANFARE, GAME_OVER, AREA_CLEAR}
        {
            SongSlotID::ITEM_CATCH,
            SongSlotID::SMALL_ITEM_CATCH,
            SongSlotID::GET_HEART_CONTAINER,
            SongSlotID::GET_MASK,
            SongSlotID::GET_OCARINA,
            SongSlotID::BOSS_CLEAR,
            SongSlotID::SNOWHEAD_CLEAR,
            SongSlotID::SOUTHERN_SWAMP_CLEARS,
            SongSlotID::EVENT_CLEAR,
            SongSlotID::GORON_RACE_GOAL,
            SongSlotID::HORSE_RACE_GOAL,
            SongSlotID::WOODFALL_RISES,
            SongSlotID::OPEN_TREASURE_BOX,
            SongSlotID::MOONS_DESTRUCTION,
            SongSlotID::THE_GIANTS_EXIT,
            SongSlotID::GAME_OVER
        }
    }
};