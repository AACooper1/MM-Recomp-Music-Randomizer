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

    memset(&archive, 0, sizeof(mz_zip_archive)); // Gotta do this because it's still 1991    
    if (!mz_zip_reader_init_file(&archive, path.string().c_str(), 0))
    {
        logger.error << "Error reading zip file." << std::endl;
        return false;
    }

    const auto time = fs::last_write_time(path).time_since_epoch();
    this->timestamp = std::chrono::duration_cast<std::chrono::seconds>(time).count();

    switch (type)
    {
        case TrackType::MMRS:
            read_from_mmrs();
            break;
        case TrackType::OOTRS:
        case TrackType::STREAMED:
        case TrackType::UNKNOWN:
        default:
            logger.error << "Unexpected filetype for " << name << ". Currently, only MMRS is supported." << std::endl;
            return false;
    }

    return true;
}

bool Track::read_from_mmrs()
{
    logger.dev << "Reading track \"" << name << "\"" << std::endl;
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

        if (filename.ends_with(".zseq") || filename.ends_with(".seq"))
        {
            sequence = std::make_shared<Sequence>(filebuffer, filename);
            try 
            {
                bankNo = std::stoi(filename, 0, 16);
            }
            catch (const std::exception& e)
            {
                logger.error << "Could not parse int for zseq " << filename << " in MMRS " << this->name << ". Song will be skipped." << std:: endl;
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
            sounds.push_back(sound);
            if (!sound->parse_foreignKey())
            {
                logger.error << "Could not parse zsound " << filename << " in track " << this->name << ", skipping!" << std::endl;
            }
        }
        else if (filename.ends_with(".mp3") || filename.ends_with(".ogg") || filename.ends_with("wav"))
        {
            logger.error << "Streamed music is not yet implemented." << std::endl;
            continue;
        }
        else if (filename == "categories.txt")
        {
            parse_categories(*filebuffer);
        }
        else if (filename.ends_with(".formmask"))
        {
            formmask.parse_file(*filebuffer);
        }
    }

    return true;
}

void Track::parse_categories(std::vector<char>& filebuffer)
{
    std::string cats_txt(filebuffer.begin(), filebuffer.end());

    std::vector<std::string> categories = split_string(cats_txt, ",-");

    for (int i = 0; i < categories.size(); i++)
    {
        int cat = -1;
        try 
        {
            cat = std::stoi(categories[i], 0, 16);
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
    (*this->categories)[id + 0x100] = true;
}