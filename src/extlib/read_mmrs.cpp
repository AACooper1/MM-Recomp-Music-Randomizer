// Project files
#include "read_mmrs.h"
#include "mmrsSql.h"

// C/C++ std Libraries
#include <stdexcept>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

// C/C++ external libraries

#define my_max(a, b) (((a) > (b)) ? (a) : (b))
#define my_min(a, b) (((a) < (b)) ? (a) : (b))

sqlite3 *db;

extern "C" 
{
    DLLEXPORT uint32_t recomp_api_version = 1;
}

bool read_mmrs(fs::directory_entry file)
{
    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Calling read_mmrs on file " << file.path().string() << "!";
    mmrs_util::debug() << END_PARA;

    // mz vars, defined here so we have access outside the try block
    mz_zip_archive mz_archive;
    mz_zip_error mz_error;
    mz_bool mz_status;

    bool success;

    fs::path in_path = file.path();

    MMRS mmrs;
    Zseq zseq;

    bool found_bankmeta = false;
    bool found_zbank = false;

    std::vector<unsigned char> zbankBuffer;
    std::vector<unsigned char> bankmetaBuffer;    

    for (int i = 0; i < MAX_DATA_SIZE; i++)
    {
        zseq.data[i] = 0xFF;
    }

    try 
    {
        int zip_filesize = fs::file_size(in_path);
        std::string zip_filename = in_path.filename().stem().string();

        mmrs_util::debug() << "File size: " << zip_filesize << "\n";

        for (int i = 0; i < zip_filename.length(); i++) 
        {
            mmrs.songName[i] = zip_filename[i];
        }

        mmrs.songName[zip_filename.length()] = '\0';

        // Clear the mz_zip_archive struct then try to read file
        memset(&mz_archive, 0, sizeof(mz_archive));

        std::string inpathStr = in_path.string();

        // Try to read the ZIP file
        mz_status = mz_zip_reader_init_file(&mz_archive, inpathStr.c_str(), 0);
        if (!mz_status)
        {
            throw std::runtime_error("Could not init zip file");
        }

        int num_files = (int)mz_zip_reader_get_num_files(&mz_archive);

        // Hooray!
        mmrs_util::debug() << "Successfully opened ZIP with " << num_files << " files!\n";

        for (int i = 0; i < num_files; i++) 
        {
            mz_zip_archive_file_stat stat;

            mz_status = mz_zip_reader_file_stat(&mz_archive, i, &stat);
            if (!mz_status) 
            {
                throw std::runtime_error("Could not read file");
            }

            // Create a new buffer instead of extracting to heap so that the memory will automatically be freed
            int filesize = stat.m_uncomp_size;
            std::vector<char> filebuffer(filesize);

            std::string filename = stat.m_filename;
            mmrs_util::debug() << "\nReading file " <<  filename << " with size " << filesize << "\n";

            mz_status = mz_zip_reader_extract_to_mem(&mz_archive, i, filebuffer.data(), filebuffer.size(), 0);
            if (!mz_status) 
            {
                throw std::runtime_error("mz_zip_reader_extract_to_mem() failed");
            }

            if (filename.ends_with(".zseq") || filename.ends_with(".seq")) 
            {
                mmrs_util::debug() << "Reading sequence file\n";

                // Make sure the extraction really succeeded.
                mmrs_util::debug() << "Data was: ";
                for (int j = 0; j < 16; j++) 
                {
                    if (j >= filebuffer.size()) break;
                    printf("%hhx ", filebuffer[j]);
                }
                mmrs_util::debug() << "...\n";

                if ((unsigned char)filebuffer.at(0) != 0xD3 || (unsigned char)filebuffer.at(1) != 0x20) 
                {
                    throw std::runtime_error("Invalid zseq header");
                }

                if (filesize > MAX_DATA_SIZE)
                {
                    throw std::runtime_error("File is too large - max 32 KiB");
                }

                for (int j = 0; j < filesize; j++) 
                {
                    zseq.data[j] = (unsigned char)filebuffer[j];
                }

                mmrs_util::debug() << "Successfully read!\n";

                mmrs.bankNo = std::stoi(filename, 0, 16);
                zseq.size = filesize;
            }
            else if (filename == "categories.txt") 
            {
                mmrs_util::debug() << "Reading categories.txt file\n";
                mmrs_util::debug() << "Categories: ";

                char* c = std::strtok(filebuffer.data(), ",");

                for(int j = 0; j < 256; j++)
                {
                    mmrs.categories[j] = false;
                }

                while (c != nullptr) 
                {
                    int cat = std::stoi(std::string(c));
                    if (cat < 256) 
                    {
                        mmrs.categories[cat] = true;
                        mmrs_util::debug() << cat << " ";
                    }
                    c = std::strtok(nullptr, ",");
                }
                mmrs_util::debug() << "\n";
            }
            else if (filename.ends_with(".zbank")) 
            {
                found_zbank = true;
                mmrs_util::debug() << "Reading zbank file" << std::endl;

                zbankBuffer.resize(filesize);

                // Make sure the extraction really succeeded.
                mmrs_util::debug() << "Data was: ";
                for (int j = 0; j < 16; j++) 
                {
                    if (j >= filebuffer.size()) break;
                    printf("%hhx ", filebuffer[j]);
                }
                mmrs_util::debug() << "...\n";

                if (filesize > MAX_DATA_SIZE)
                {
                    throw std::runtime_error("File is too large - max 32 KiB");
                }

                for (int j = 0; j < filesize; j++) 
                {
                    zbankBuffer[j] = (unsigned char)filebuffer[j];
                }

                mmrs_util::debug() << "Successfully read!\n";
            }
            else if (filename.ends_with(".bankmeta")) 
            {
                found_bankmeta = true;
                mmrs_util::debug() << "Reading zbank file" << std::endl;

                bankmetaBuffer.resize(filesize);

                // Make sure the extraction really succeeded.
                mmrs_util::debug() << "Data was: ";
                for (int j = 0; j < 16; j++) 
                {
                    if (j >= filebuffer.size()) break;
                    printf("%hhx ", filebuffer[j]);
                }
                mmrs_util::debug() << "...\n";

                if (filesize > MAX_DATA_SIZE)
                {
                    throw std::runtime_error("File is too large - max 32 KiB");
                }

                for (int j = 0; j < filebuffer.size(); j++) 
                {
                    if (j >= filebuffer.size()) break;
                    bankmetaBuffer[j] = (unsigned char)filebuffer[j];
                }

                mmrs_util::debug() << "Successfully read!\n";
            }
            else 
            {
                mmrs_util::debug() << "Unknown filetype " << filename << std::endl;
            }
        }

        success = true;
        // Update the database
        int mmrsId = insert_mmrs(mmrs, zseq, file);
        
        if (found_zbank && found_bankmeta)
        {
            Zbank zbank;
            zbank.metaSize = bankmetaBuffer.size();
            zbank.bankSize = zbankBuffer.size();
            // Initialize to 255 ("End of data")
            for (int j = 0; j < MAX_DATA_SIZE; j++)
            {
                zbank.bankData[j] = '\0';
                zbank.metaData[j] = '\0';
            }
            
            for (int j = 0; j < zbank.bankSize; j++) 
            {
                zbank.bankData[j] = zbankBuffer[j];
            }
            for (int j = 0; j < zbank.metaSize; j++) 
            {
                zbank.metaData[j] = bankmetaBuffer[j];
            }
            success = insert_zbank(zbank, mmrsId);
        }
        else if (found_zbank ^ found_bankmeta)
        {
            mmrs_util::warning() << "Warning: Found one of .zbank or .bankmeta, but not both for file " << zip_filename << "!" << std::endl;
            mmrs_util::warning() << ".bankmeta: " << found_bankmeta << std::endl;
            mmrs_util::warning() << ".zbank: " << found_zbank << std::endl;
        }
    } 
    catch (const std::exception &e) 
    {
        std::cerr << "MMRS Read error: " << e.what() << "\n";
        success = false;
    } 
    catch (...) 
    {
        std::cerr << "MMRS Read error: Unknown error\n";
        success = false;
    }

    mz_error = mz_zip_get_last_error(&mz_archive);
    if (mz_error != MZ_ZIP_NO_ERROR) 
    {
        mmrs_util::debug() << "mz_error: " << mz_zip_get_error_string(mz_error) << "\n";
    }

    mmrs_util::debug() << "================================================================================\n\n";

    // Cleanup
    mz_zip_reader_end(&mz_archive);

    // Return true or false
    return success;
}

int read_seq_directory(const char* dbPath)
{
    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Calling read_seq_directory!";
    mmrs_util::debug() << END_PARA;

    const fs::path dir = "music";

    int status = 0;

    char *sqlErrorMsg;
    int sqlErrCode;
    std::unordered_set<std::string> filenames;

    if(fs::exists(dir)) 
    {
        int i = 0;
        for(const fs::directory_entry entry: fs::directory_iterator(dir)) {
            printf("i: %d\n\n", i);

            std::string filename = entry.path().filename().string();
            std::string songNameStr = entry.path().filename().stem().string();
            const char* songName = songNameStr.c_str();

            // If file has an extension other than .mmrs, print that to the console and continue
            if (entry.path().extension() != ".mmrs") 
            {
                printf("File %s is not a .mmrs file, skipping.", filename.c_str());
            }
            else 
            {
                filenames.insert(filename);
                if (check_mmrs_exists(entry))
                {
                    i++;
                    continue;
                }
                else 
                {
                    mmrs_util::debug() << fs::absolute(entry.path()) << std::endl;

                    bool success = read_mmrs(entry);

                    if (!success) 
                    {
                        mmrs_util::error() << "Could not read file " << entry.path().filename().string();
                        continue;
                    }

                    mmrs_util::debug() << "Successfully read file " << entry.path().filename().string() << std::endl;
                }
                i++;
            }
        }

        mmrs_util::debug() << std::endl;
        for (auto& it: filenames)
        {
            mmrs_util::debug() << it << std::endl;
        }

        // Now remove any that are in the db but not the folder
        int mmrsCount = count_mmrs();
        std::string dbFiles[mmrsCount];
        int dbIds[mmrsCount];
        
        retrieve_filenames(dbIds, dbFiles);

        bool success = false;

        for (int j = 0; j < mmrsCount; j++)
        {
            if (! filenames.contains(dbFiles[j]))
            {
                mmrs_util::debug() << "DB entry " << dbFiles[j] << " not in music folder. Deleting..." << std::endl;
                success = remove_mmrs(dbIds[j]);
            }
        }

        return count_mmrs();
    } 
    else 
    {
        mmrs_util::error() << "\ndir" << fs::current_path().string() << "\\" << dir.string() << "does not exist.\n";
        return -2;
    }

    return -1;
}

RECOMP_DLL_FUNC(sql_init)
{
    std::string dbPathStr = RECOMP_ARG_STR(0);
    const char* dbPath = dbPathStr.c_str();

    printf("%s\n", dbPath);

    if(_sql_init(dbPath))
    {
        RECOMP_RETURN(bool, true);
    }
    else
    {
        RECOMP_RETURN(bool, false);
    }
}

/*
    read_mmrs_files():
        Reads the music directory and updates the database.
        Does not fill the mod RAM table, which is handled by load_mmrs_table.

        Parameters:
            const char* dbPath: The path of the database to update.

        Returns:
            int numMmrs:        The number of entries in the MMRS table.
*/

RECOMP_DLL_FUNC(read_mmrs_files)
{
    mmrs_util::set_log_level(mmrs_util::LOG_DEBUG);
    
    if (mmrs_util::gLogLevel >= mmrs_util::LOG_DEBUG)
    {
        printf(START_PARA);
        printf("START EXTLIB");
        printf(END_PARA);
    }

    std::string dbPathStr = RECOMP_ARG_STR(0);
    const char *dbPath = dbPathStr.c_str();
    
    bool initDb = false;
    
    printf("Calling init_mmrs_cache!");
    try
    {
        initDb = init_mmrs_cache();
    }
    catch (std::exception e)
    {

            printf("Error initalizing music DB: %s\n", e.what());
        RECOMP_RETURN(int, -1);
    }
    
    int numMmrs = read_seq_directory(dbPath);

    if (mmrs_util::gLogLevel >= mmrs_util::LOG_DEBUG)
    {
        printf("Completed extlib function read_mmrs_files(). Number of MMRSes: %i", numMmrs);
        printf(END_PARA);
    }

    RECOMP_RETURN(int, numMmrs);
}

/*
    load_mmrs_table()
        Loads the MMRS table into mod memory from the DB.
        Does not load Zseq and Zbank info, which are only pulled when necessary.

        Parameters:
            const char* dbPath  :   Path to the database.
            MMRS* allMmrs       :   Address of the MMRS table in mod memory.
            int numMmrs         :   Number of entries in the MMRS table.

        Returns:
            bool success        :   Whether the load succeeded.
*/

RECOMP_DLL_FUNC(load_mmrs_table)
{
    MMRS* allMmrs = RECOMP_ARG(MMRS*, 0);

    bool success = _load_mmrs_table(allMmrs);

    if(success)
    {
        for (int n = 0; n < strlen(allMmrs[0].songName); n++)
        {
            std::cout << allMmrs[0].songName[n];
        }
        std::cout << std::endl;
        RECOMP_RETURN(bool, false);
    }
    else
    {
        RECOMP_RETURN(bool, true);
    }
}

RECOMP_DLL_FUNC(load_zseq)
{
    Zseq* zseqAddr = RECOMP_ARG(Zseq*, 0);
    int zseqId = RECOMP_ARG(int, 1);

    bool success = _load_zseq(zseqAddr, zseqId);

    if(success)
    {
        RECOMP_RETURN(bool, true);
    }
    else
    {
        RECOMP_RETURN(bool, false);
    }
}

RECOMP_DLL_FUNC(load_zbank)
{
    Zbank* zbankAddr = RECOMP_ARG(Zbank*, 0);
    int zbankId = RECOMP_ARG(int, 1);

    bool success = _load_zbank(zbankAddr, zbankId);
    
    if(success)
    {
        RECOMP_RETURN(bool, true);
    }
    else
    {
        RECOMP_RETURN(bool, false);
    }
}

RECOMP_DLL_FUNC(sql_teardown)
{
    if(_sql_teardown())
    {
        RECOMP_RETURN(bool, true);
    }
    else
    {
        RECOMP_RETURN(bool, false);
    }
}