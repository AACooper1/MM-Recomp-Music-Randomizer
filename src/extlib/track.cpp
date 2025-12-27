#include "track.h"

Track::Track(fs::path path)
{
    id = 0;
    this->path = path;
    name = path.stem().string();

    if (path.extension() == "mmrs")
    {
        type = TrackType::MMRS;
    }
    else if (path.extension() == "ootrs")
    {
        type = TrackType::OOTRS;
    }
    else
    {
        type = TrackType::UNKNOWN;
    }

    sequence = new Sequence;
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
    logger.debug << "Detected file type MMRS." << std::endl;

    int num_files = (int)mz_zip_reader_get_num_files(&archive);
    for (int i = 0; i < num_files; i++)
    {
        mz_zip_archive_file_stat stat;

        if (mz_zip_reader_file_stat(&archive, i, &stat))
        {
            logger.error << "Error reading file " << stat.m_filename << ", skipping!" << std::endl;
        }

        int filesize = stat.m_uncomp_size;
        std::vector<char> filebuffer(filesize);

        if (!mz_zip_reader_extract_to_mem(&archive, i, filebuffer.data(), filebuffer.size(), 0))
        {
            throw std::runtime_error("mz_zip_reader_extract_to_mem() failed");
        }

        std::string filename = stat.m_filename;
        AudioFileFactory factory;

        AudioFile* file = factory.read_file(filename, filebuffer);
    }
}