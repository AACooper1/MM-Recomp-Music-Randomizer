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