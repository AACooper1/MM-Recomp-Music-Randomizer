#include "track.h"

Track::Track(fs::path path)
{
    databaseIndex = 0;
    this->path = path;
    categories = std::make_unique<std::vector<char>>();
    categories->resize(0x200);
    name = path.stem().string();

    if (path.extension() == ".mmrs")
    {
        type = TrackType::MMRS;
    }
    else if (path.extension() == ".ootrs")
    {
        type = TrackType::OOTRS;
    }
    else if (path.extension() == ".zseq")
    {
        type = TrackType::ZSEQ;
    }
    else
    {
        type = TrackType::UNKNOWN;
    }
    
    sequence = nullptr;
    bank = nullptr;
    bankNo = 0;
}

Track::Track()
{
    databaseIndex = 0;
    path = "";
    categories = std::make_unique<std::vector<char>>(0x200, false);
    name = "";
    bankNo = 0;

    sequence = nullptr;
    bank = nullptr;
}

bool Track::read_from_file()
{
    logger.debug << "Reading file " << path.filename() << "..." << std::endl;

    if (!fs::exists(path))
    {
        logger.error << "Error: " << path.string() << "does not exist!" << std::endl;
    }
    
    const auto time = fs::last_write_time(path).time_since_epoch();
    this->timestamp = std::chrono::duration_cast<std::chrono::seconds>(time).count();
    
    if (type == TrackType::ZSEQ)
    {
        return read_from_zseq();
    }

    memset(&archive, 0, sizeof(mz_zip_archive)); // Gotta do this because it's still 1991
    std::u8string u8path = path.u8string();
    if (!mz_zip_reader_init_file(&archive, reinterpret_cast<const char*>(u8path.c_str()), 0))
    {
        logger.error << "Error reading zip file " << path.string() << ": " << mz_zip_get_error_string(mz_zip_get_last_error(&archive)) << std::endl;
        mz_zip_reader_end(&archive);
        return false;
    }

    switch (type)
    {
        case TrackType::MMRS:
            read_from_mmrs();
            break;
        case TrackType::OOTRS:
            read_from_ootrs();
            break;
        case TrackType::STREAMED:
        case TrackType::UNKNOWN:
        default:
            logger.error << "Unexpected filetype for " << name << ". Currently, only MMRS is supported." << std::endl;
            return false;
    }

    mz_zip_reader_end(&archive);
    return true;
}

bool Track::read_from_zseq()
{
    logger.debug << "Detected file type zseq." << std::endl;
    std::string filename = path.stem().string();
    std::vector<std::string> split_filename = split_string(filename, "_");
    
    if (split_filename.size() != 3)
    {
        logger.error << "Zseq " << name << " had " << split_filename.size() << " fields, expected 3!" << std::endl;
        return false;
    }

    this->name = split_filename[0];
    try
    {
        this->bankNo = std::stoi(split_filename[1]);
    }
    catch (std::exception& e)
    {
        logger.error << "Could not parse int for seq " << filename << " in zseq " << this->name << ". Song will be skipped." << std:: endl;
        return false;
    }

    std::vector<char> _categories;
    _categories.assign(split_filename[2].begin(), split_filename[2].end());
    parse_categories(_categories);

    std::ifstream zseq_file(path, std::ios::binary);
    if (!zseq_file.is_open())
    {
        logger.error << "Could not open zseq " << name << ". Song will be skipped." << std::endl;
        return false;
    }

    zseq_file.unsetf(std::ios::skipws);
    std::streampos filesize;
    zseq_file.seekg(0, std::ios::end);
    filesize = zseq_file.tellg();
    zseq_file.seekg(0, std::ios::beg);

    std::shared_ptr<std::vector<char>> filebuffer = std::make_shared<std::vector<char>>(filesize);
    zseq_file.read(filebuffer->data(), filesize);

    this->sequence = std::make_shared<Sequence>(filebuffer, filename);

    return true;
}

bool Track::read_from_mmrs()
{
    logger.debug << "Detected file type MMRS." << std::endl;

    int num_files = (int)mz_zip_reader_get_num_files(&archive);
    
    for (int i = 0; i < num_files; i++)
    {
        mz_zip_archive_file_stat stat;

        if (!mz_zip_reader_file_stat(&archive, i, &stat))
        {
            logger.error << "Error reading file " << stat.m_filename << ", skipping!" << std::endl;
            continue;
        }

        int filesize = stat.m_uncomp_size;
        std::shared_ptr<std::vector<char>> filebuffer = std::make_shared<std::vector<char>>(filesize);

        if (!mz_zip_reader_extract_to_mem(&archive, i, filebuffer->data(), filebuffer->size(), 0))
        {
            throw std::runtime_error("mz_zip_reader_extract_to_mem() failed");
        }

        std::string filename = stat.m_filename;
        if (fs::path(filename).extension().string().ends_with("seq"))
        {
            sequence = std::make_shared<Sequence>(filebuffer, filename);
            try 
            {
                bankNo = std::stoi(filename, 0, 16);
            }
            catch (const std::exception& e)
            {
                logger.error << "Could not parse int for seq " << filename << " in MMRS " << this->name << ". Song will be skipped." << std:: endl;
                return false;
            }
        }
        else if (filename.ends_with(".zbank"))
        {
            if (!bank)
                bank = std::make_shared<Bank>(filebuffer, filename, false);
            else
                bank->read_from_file(filebuffer);
        }
        else if (filename.ends_with(".bankmeta"))
        {
            if (!bank)
                bank = std::make_shared<Bank>(filebuffer, filename, true);
            else
                bank->read_header(filebuffer);
        }
        else if (filename.ends_with(".zsound"))
        {
            std::shared_ptr<Sound> sound = std::make_shared<Sound>(filebuffer, filename);
            if (!sound->parse_foreignKey())
            {
                logger.error << "Could not parse zsound " << filename << " in MMRS " << this->name << ", skipping!" << std::endl;
                return false;
            }
            sounds.push_back(sound);

        }
        else if (filename.ends_with(".mp3") || filename.ends_with(".ogg") || filename.ends_with("wav"))
        {
            // logger.error << "Streamed music is not yet implemented." << std::endl;
            continue;
        }
        else if (filename == "categories.txt")
        {
            parse_categories(*filebuffer);
        }
        else if (filename.ends_with(".formmask"))
        {
            formmask.parse_file(*filebuffer);
            formmask.is_default = false;
        }
    }

    if (!sequence && type != TrackType::STREAMED)
    {
        logger.error << "Non-streamed track " << name << " did not have a sequence and will not be added to the database!" << std::endl;
    }

    return true;
}

bool Track::read_from_ootrs()
{
    logger.debug << "Detected file type OOTRS." << std::endl;

    int num_files = (int)mz_zip_reader_get_num_files(&archive);

    for (int i = 0; i < num_files; i++)
    {
        mz_zip_archive_file_stat stat;

        if (!mz_zip_reader_file_stat(&archive, i, &stat))
        {
            logger.error << "Error reading file " << stat.m_filename << ", skipping!" << std::endl;
            continue;
        }

        int filesize = stat.m_uncomp_size;
        std::shared_ptr<std::vector<char>> filebuffer = std::make_shared<std::vector<char>>(filesize);

        if (!mz_zip_reader_extract_to_mem(&archive, i, filebuffer->data(), filebuffer->size(), 0))
        {
            throw std::runtime_error("mz_zip_reader_extract_to_mem() failed");
        }

        std::string filename = stat.m_filename;
        if (fs::path(filename).extension().string().ends_with("seq"))
        {
            sequence = std::make_shared<Sequence>(filebuffer, filename);
        }
        else if (filename.ends_with(".zbank"))
        {
            if (!bank)
                bank = std::make_shared<Bank>(filebuffer, filename, false);
            else
                bank->read_from_file(filebuffer);
        }
        else if (filename.ends_with(".bankmeta"))
        {
            if (!bank)
                bank = std::make_shared<Bank>(filebuffer, filename, true);
            else
                bank->read_header(filebuffer);
        }
        else if (filename.ends_with(".zsound"))
        {
            bool found = false;
            for (int j = 0; j < sounds.size(); j++)
            {
                if (sounds[j]->filename == filename)
                {
                    sounds[j]->data = filebuffer;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::shared_ptr<Sound> sound = std::make_shared<Sound>(filebuffer, filename);
                sounds.push_back(sound);
            }

        }
        else if(filename.ends_with(".meta"))
        {
            parse_meta(*filebuffer);
        }
    }

    for (int i = 0; i < sounds.size(); i++)
    {
        if (!sounds[i]->sampleAddr)
        {
            logger.warning << "Zsound " << sounds[i]->filename << " in OOTRS " << name << " is missing sampleAddr, will not be found in game!" << std::endl;
        }
        else if (!sounds[i]->data)
        {
            logger.error << "Zsound " << sounds[i]->filename << " in OOTRS " << name << " has no data! Song will be skipped!" << std::endl;
            return false;
        }
    }

    return true;
}

bool Track::parse_meta(std::vector<char>& filebuffer)
{
    if (type != TrackType::OOTRS)
    {
        logger.error << "Tried to use parse_meta on a non-OOTRS track?!?!" << std::endl;
    }
    std::string meta_txt(filebuffer.begin(), filebuffer.end());

    std::vector<std::string> lines = split_string(meta_txt, "\n");

    if (lines.size() < 2)
    {
        logger.error << ".meta file for track " << name << " has only " << lines.size() << " lines, skipping!" << std::endl;
        return false;
    }
    else
    {
        name = lines[0];
        if (lines[0].starts_with("-"))
        {
            bankNo = 0x28;
        }
        else
        {
            try
            {
                bankNo = std::stoi(lines[1], 0, 16);
            }
            catch (const std::exception& e)
            {
                logger.error << "Could not parse int for bank \"" << lines[1] << "\" in OOTRS " << this->name << ". Song will be skipped." << std:: endl;
                return false;
            }
        }
        std::string metaCategories = "";
        for (int i = 2; i < lines.size(); i++)
        {
            metaCategories += lines[i] + '\n';
        }
        parse_oot_categories(metaCategories);

        for (int i = 2; i < lines.size(); i++)
        {
            if (lines[i].starts_with("ZSOUND"))
            {
                std::vector<std::string> line = split_string(lines[i], ":");
                std::string filename = line[1];
                u32 sampleAddr = std::stoll(line[2], 0, 16);
                bool found = false;
                for (int j = 0; j < sounds.size(); j++)
                {
                    if (sounds[j]->filename == filename)
                    {
                        sounds[j]->sampleAddr = sampleAddr;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    std::shared_ptr<Sound> sound = std::make_shared<Sound>();
                    sound->filename = filename;
                    sound->sampleAddr = sampleAddr;

                    sounds.push_back(sound);
                }
                
            }
        }
    }

    return true;
}

bool Track::fix_oot_custom_bank(OoTAudioHandler* ootAudioHandler)
{
    if (type == TrackType::OOTRS && bank)
    {
        std::shared_ptr<std::vector<u8>> header = std::make_shared<std::vector<u8>>((*bank->header).begin(), (*bank->header).end());
        std::shared_ptr<std::vector<u8>> data = std::make_shared<std::vector<u8>>((*bank->data).begin(), (*bank->data).end());
        AudioBank parsed_bank = ootAudioHandler->match_custom_oot_bank(header, data);

        bank = std::make_shared<Bank>(parsed_bank);
        for (int i = 0; i < parsed_bank.zsounds_to_add.size(); i++)
        {
            std::shared_ptr<Sound> sound = std::make_shared<Sound>(parsed_bank.zsounds_to_add[i]);
            sounds.push_back(sound);
        }
    }

    return true;
}

void Track::parse_oot_categories(std::string metaCategories)
{
    _is_fanfare = metaCategories.contains("fanfare") || metaCategories.contains("Fanfare");

    if (metaCategories.length() == 0 || metaCategories == "bgm" || metaCategories == "bgm\n")
    {
          (*this->categories)[FIELD]
        = (*this->categories)[TOWN]
        = (*this->categories)[DUNGEON]
        = (*this->categories)[INDOORS]
        = (*this->categories)[MINIGAME]
        = true;
        return;
    }
    else if (metaCategories == "fanfare" || metaCategories == "fanfare\n")
    {
          (*this->categories)[FANFARE]
        = (*this->categories)[GAME_OVER]
        = (*this->categories)[AREA_CLEAR]
        = true;
        return;
    }
    else
    {
        std::vector<std::string> cats = split_string(metaCategories, ",\n");

        for (int i = 0; i < cats.size(); i++)
        {
            if (OoTBGMGroupsToCategories.contains(cats[i]))
            {
                std::vector<SongSlotID>& this_category = OoTBGMGroupsToCategories.at(cats[i]);
                for(int c = 0; c < this_category.size(); c++)
                {
                    (*this->categories)[(int)this_category[c]] = true;
                }
            }
        }
    }
}

void Track::parse_categories(std::vector<char>& filebuffer)
{
    if (type == TrackType::OOTRS)
    {
        logger.error << "Tried to use parse_categories instead of parse_meta for OOTRS?!?!" << std::endl;
    }
    std::string cats_txt(filebuffer.begin(), filebuffer.end());

    std::vector<std::string> categories = split_string(cats_txt, ",-");

    for (int i = 0; i < categories.size(); i++)
    {
        int cat = -1;
        try 
        {
            cat = std::stoi(categories[i], 0, 16);
            if (cat == 0x10) { cat = 10; }  // Special handling for these two, as MMR handles
            if (cat == 0x16) {cat = 16; }   // song slots in base 16 but categories in base 10
        }
        catch (const std::exception& e) 
        {
            logger.error << "Could not parse int for category \"" << categories[i] << "\" in track " << name << std:: endl;
            continue;
        }

        if (cat < 0x200)
        {
            (*this->categories)[cat] = true;
        }
        else
        {
            logger.warning << "Track " << name << "had category " << cat << ", which is not allowed." << std::endl;
        }
    }

    return;
}


/* Used for vanilla tracks */
void Track::parse_categories(const std::vector<int>& categories)
{
    for (int i = 0; i < categories.size(); i++)
    {
        if (categories[i] < 0x200)
        {
            (*this->categories)[categories[i]] = true;
        }
        else
        {
            logger.warning << "Vanila track " << name << "had category " << categories[i] << ", which is not allowed." << std::endl;
        }
    }
    (*this->categories)[id] = true;
}