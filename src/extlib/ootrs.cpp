#include "ootrs.hpp"

OoTAudioHandler::OoTAudioHandler(fs::path path)
{
    this->audiobinPath = path;
    this->mmRomPath = path.parent_path().parent_path()/"mm.n64.us.1.0.z64";
}

void OoTAudioHandler::prepare_oot_audio()
{
    unzip_oot_audiobin();
    copy_mm_rom();
    get_mm_files();
    get_all_banks();
    get_all_mm_samples();
    match_all_oot_banks();
    logger.debug << "Matched OoT audio!" << std::endl;
}

bool OoTAudioHandler::unzip_oot_audiobin()
{
    memset(&this->archive, 0, sizeof(mz_zip_archive));
    if (!mz_zip_reader_init_file(&archive, audiobinPath.string().c_str(), 0))
    {
        logger.error << "Error reading zip file " << audiobinPath.string() << ": " << mz_zip_get_error_string(mz_zip_get_last_error(&archive)) << std::endl;
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
                std::vector<u8> this_file(filesize);
                for (int j = 0; j < filesize; j++)
                {
                    this_file[j] = filebuffer.data()[j];
                }

                ootFilesRaw.push_back(this_file);                
                ootFiles.insert(std::make_pair(filename, ByteArray(ootFilesRaw[ootFilesRaw.size() - 1])));
            }
            for (int i = 0; i < 4; i++)
            {
                if (!ootFiles.contains(expected_files[i]))
                {
                    logger.error << "Audiobin did not contain " << expected_files[i] << ", aborting!" << std::endl;                    
                    successfully_parsed = false;
                    return false;
                }
                else
                {
                    logger.debug << "Found file " << expected_files[i] << "!" << std::endl;
                }
            }
            
            successfully_parsed = true;
        }
    }

    mz_zip_reader_end(&archive);

    return true;
}

bool OoTAudioHandler::copy_mm_rom()
{
    if (!fs::exists(mmRomPath))
    {
        logger.error << "Could not find MM ROM?!" << std::endl;
        return false;
    }
    else
    {
        logger.debug << "Reading and decompressing MM ROM! This may take a second..." << std::endl;

        // Allocates vector in advance. ihatethatiusedaitooptimizethisineedtostop:(
        std::ifstream in(mmRomPath, std::ios::binary);
        in.seekg(0, std::ios::end);
        std::streamsize size = in.tellg();
        in.seekg(0, std::ios::beg);

        std::vector<u8> compressed_rom(size);
        in.read(reinterpret_cast<char*>(compressed_rom.data()), size);

        ByteArray compressed_rom_span(compressed_rom);

        mmRomRaw = decompress_rom(compressed_rom_span);
        SHA1 checksum;
        checksum.update(std::string(mmRomRaw.begin(), mmRomRaw.end()));
        std::string checksum_final = checksum.final();
        assert(checksum_final == "e5cecc349b49b6963cce200298e932daa641d1fc");
        logger.debug << "Finished reading and decompressing MM ROM, SHA1s match!" << std::endl;
        mmRom = mmRomRaw;

        return true;
    }
}

bool OoTAudioHandler::get_mm_files()
{
    mmFiles.insert(std::make_pair(AUDIOTABLE_HEADER, mmRom.subspan(MM_AUDIOTABLE_HEADER_OFFSET, MM_AUDIOTABLE_HEADER_LENGTH)));
    mmFiles.insert(std::make_pair(BANKTABLE_HEADER, mmRom.subspan(MM_BANKTABLE_HEADER_OFFSET, MM_BANKTABLE_HEADER_LENGTH)));
    mmFiles.insert(std::make_pair(AUDIOTABLE, mmRom.subspan(MM_AUDIOTABLE_OFFSET, MM_AUDIOTABLE_LENGTH)));
    mmFiles.insert(std::make_pair(BANKTABLE, mmRom.subspan(MM_BANKTABLE_OFFSET, MM_BANKTABLE_LENGTH)));

    return true;
}

bool OoTAudioHandler::get_all_banks()
{
    this->numOoTBanks = int16_from_bytes(this->ootFiles[BANKTABLE_HEADER], 0);
    for (int i = 0; i < numOoTBanks; i++)
    {
        int offset = 0x10 + (0x10 * i);
        std::span<u8> curr_entry = this->ootFiles[BANKTABLE_HEADER].subspan(offset, 0x10);
        AudioBank audiobank = AudioBank(i, curr_entry, this->ootFiles[BANKTABLE], this->ootFiles[AUDIOTABLE], this->ootFiles[AUDIOTABLE_HEADER]);
        this->ootBanks.push_back(audiobank);
    }

    this->numMMBanks = int16_from_bytes(this->mmFiles[BANKTABLE_HEADER], 0);
    for (int i = 0; i < numMMBanks; i++)
    {
        int offset = 0x10 + (0x10 * i);
        std::span<u8> curr_entry = this->mmFiles[BANKTABLE_HEADER].subspan(offset, 0x10);
        AudioBank audiobank = AudioBank(i, curr_entry, this->mmFiles[BANKTABLE], this->mmFiles[AUDIOTABLE], this->mmFiles[AUDIOTABLE_HEADER]);
        this->mmBanks.push_back(audiobank);
    }

    return true;
}

void OoTAudioHandler::get_all_mm_samples()
{
    for (int i = 0; i < numMMBanks; i++)
    {
        AudioBank& bank = mmBanks[i];
        std::vector<Sample*> bankSamples = bank.get_all_samples();
        all_mm_samples.insert(all_mm_samples.end(), bankSamples.begin(), bankSamples.end());
    }
}

int OoTAudioHandler::find_sample_in_mm_banks(ByteArray sample_data, int audioTableId)
{
    if (sample_data.size() <= 0)
    {
        return -1;
    }
    for (int sampleIdx = 0; sampleIdx < this->all_mm_samples.size(); sampleIdx++)
    {
        ByteArray sampleData = all_mm_samples[sampleIdx]->data;
        if (sampleData.size() <= 0)
        {
            continue;
        }
        if (!std::memcmp(sample_data.data(), sampleData.data(), std::min(sample_data.size(), sampleData.size())))
        {
            if (audioTableId == all_mm_samples[sampleIdx]->audiotableId)
            return all_mm_samples[sampleIdx]->sampleAddr;
        }
    }

    return -1;
}

AudioBank OoTAudioHandler::match_custom_oot_bank(std::shared_ptr<std::vector<u8>> header, std::shared_ptr<std::vector<u8>> data)
{    
    ByteArray headerArray{*header};
    ByteArray dataArray{*data};

    AudioBank bank(0x28, data->size(), headerArray, dataArray, ootFiles[AUDIOTABLE], ootFiles[AUDIOTABLE_HEADER]);

    std::vector<Sample*> allSamples = bank.get_all_samples();
    for (int sampleIdx = 0; sampleIdx < allSamples.size(); sampleIdx++)
    {
        ByteArray sampleData = allSamples[sampleIdx]->data;
        int match = find_sample_in_mm_banks(sampleData, allSamples[sampleIdx]->audiotableId);
        if (match >= 0)
        {
            allSamples[sampleIdx]->sampleAddr = match;
            int reverse_match = std::byteswap(match);
            u8* addr = &bank.bank_data[allSamples[sampleIdx]->sample_offset + 4];
            std::memcpy(addr, &reverse_match, sizeof(int));
        }
        else
        {
            bank.zsounds_to_add.push_back(allSamples[sampleIdx]);
        }
    }

    return bank;
}

bool OoTAudioHandler::match_all_oot_banks()
{
    for (int bankIdx = 0; bankIdx < this->numOoTBanks; bankIdx++)
    {
        AudioBank& bank = ootBanks[bankIdx];
        std::vector<Sample*> allSamples = bank.get_all_samples();
        for (int sampleIdx = 0; sampleIdx < allSamples.size(); sampleIdx++)
        {
            ByteArray sampleData = allSamples[sampleIdx]->data;
            int match = find_sample_in_mm_banks(sampleData, allSamples[sampleIdx]->audiotableId);
            if (match >= 0)
            {
                allSamples[sampleIdx]->sampleAddr = match;
                int reverse_match = std::byteswap(match);
                u8* addr = &bank.bank_data[allSamples[sampleIdx]->sample_offset + 4];
                std::memcpy(addr, &reverse_match, sizeof(int));
            }
            else
            {
                bool exists = false;
                for (int i = 0; i < bank.zsounds_to_add.size(); i++)
                {
                    if (allSamples[sampleIdx]->sampleAddr == bank.zsounds_to_add[i]->sampleAddr)
                    {
                        exists = true;
                        break;
                    }
                }
                if (!exists)
                {
                    bank.zsounds_to_add.push_back(allSamples[sampleIdx]);
                }
            }
        }
    }

    return true;
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
        "Fields", // == MMR's {FIELD} + Woods of Mystery
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
            SongSlotID::ZORA_HALL,     // Are they stupid??
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