#include "track.h"

Track::Track(fs::path path)
{
    databaseIndex = 0;
    this->path = path;
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

bool Track::read_from_file()
{
    logger.debug << "Reading file " << path.filename() << "..." << std::endl;

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
            sequence = new Sequence(filebuffer);
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
            bank = new Bank(filebuffer);
        else if (filename.ends_with(".zsound"))
            sounds.push_back(new Sound(filebuffer));
        else if (filename.ends_with(".mp3") || filename.ends_with(".ogg") || filename.ends_with("wav"))
        {
            logger.error << "Streamed music is not yet implemented." << std::endl;
            continue;
        }
        else if (filename == "categories.txt")
        {
            parse_categories(filebuffer);
        }
        else if (filename.ends_with(".formmask"))
        {
            parse_formmask(filebuffer);
        }
    }

    return true;
}

void Track::parse_categories(std::shared_ptr<std::vector<char>> filebuffer)
{
    std::string cats_txt(filebuffer->begin(), filebuffer->end());

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
            logger.error << "Could not parse int for category \"" << categories[i] << "\" in MMRS " << name << std:: endl;
            continue;
        }

        if (cat < 0x200)
        {
            categories[cat] = true;
        }
    }

    return;
}

void Track::parse_formmask(std::shared_ptr<std::vector<char>> filebuffer)
{
    std::string formmask_txt(filebuffer->begin(), filebuffer->end());

    std::vector<std::string> channels = split_string(formmask_txt, "[\"]");
    for (int i = channels.size(); i >= 0; i--)
    {
        if (i % 2 == 1)
        {
            channels.erase(channels.begin()+i-1);
        }
    }
    if (channels.size() > 17)
    {
        logger.error << "Formmask for " << name << " has more than 17 rows " << "(" << channels.size() << "), skipping!";
    }

    for (int ch = 0; ch < channels.size() && ch < 16; ch++)
    {
        std::string& line = channels[ch];
        int stateIdx = 0;

        if (line.contains("None"))
        {
            stateIdx = 5;
        }
        else if (line.contains("All"))
        {
            formmask.states[ch] += FIERCE_DEITY | GORON | ZORA | DEKU | HUMAN;
            stateIdx = 5;
        }

        for (stateIdx; stateIdx < 12; stateIdx++)
        {
            if (line.contains(playStates[stateIdx]))
                formmask.states[ch] += (1 << stateIdx);
        }
    }

    if (channels.size() == 17)
    {
        for (int stateIdx = 5; stateIdx < 12; stateIdx++)
        {
            std::string& line = channels[16];

            if (line.contains(playStates[stateIdx]))
                formmask.cumulativeStates += (1 << stateIdx);
        }
    }

    for (int c = 5; c < 13; c++)
    {
        if (!formmask_txt.contains(playStates[c]))
        {
            logger.debug << playStates[c] << " not in formmask, will be added as cumulative state..." << std::endl;
            formmask.cumulativeStates += (1 << c);
        }
    }

    return;
}