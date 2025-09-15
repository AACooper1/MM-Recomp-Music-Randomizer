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
std::unordered_map<u32, u32> zsound_key_to_pointer;

extern "C" 
{
    DLLEXPORT uint32_t recomp_api_version = 1;
}

bool read_mmrs(fs::directory_entry file)
{
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

    std::vector<Zsound> zsounds;

    for (int i = 0; i < MAX_ZSEQ_SIZE; i++)
    {
        zseq.data[i] = 0xFF;
    }

    for (int s = 0; s < 17; s++)
    {
        mmrs.formmask[s] = UINT16_MAX;
    }

    try 
    {
        int zip_filesize = fs::file_size(in_path);
        std::string zip_filename = in_path.filename().stem().string();

        for (int i = 0; i < zip_filename.length(); i++) 
        {
            mmrs.songName[i] = zip_filename[i];
        }

        // Initialize categories to 0
        for(int j = 0; j < 256; j++)
        {
            mmrs.categories[j] = false;
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

            mz_status = mz_zip_reader_extract_to_mem(&mz_archive, i, filebuffer.data(), filebuffer.size(), 0);
            if (!mz_status) 
            {
                throw std::runtime_error("mz_zip_reader_extract_to_mem() failed");
            }

            if (filename.ends_with(".zseq") || filename.ends_with(".seq")) 
            {
                if (filesize > MAX_ZSEQ_SIZE)
                {
                    throw std::runtime_error("Zseq file is too large - max 1 MiB!\n");
                }

                for (int j = 0; j < filesize; j++) 
                {
                    zseq.data[j] = (unsigned char)filebuffer[j];
                }

                try 
                {
                    mmrs.bankNo = std::stoi(filename, 0, 16);
                }
                catch(const std::exception& e)
                    {
                        mmrs_util::error() << "Could not parse int for zseq " << filename << " in MMRS " << mmrs.songName << ". Song will be skipped." << std:: endl;
                        return false;
                    }
                zseq.size = filesize;
            }
            else if (filename.ends_with(".formmask"))
            {
                filebuffer.push_back('\0');
                std::istringstream formmask(filebuffer.data());
                std::string line;
                int lineIdx = 0;

                for (int s = 0; s < 17; s++)
                {
                    mmrs.formmask[s] = 0;
                }

                std::string playStates[13] = 
                {
                    "FierceDeity",
                    "Goron",
                    "Zora",
                    "Deku",
                    "Human",
                    "Outdoors",
                    "Indoors",
                    "Cave",
                    "Epona",
                    "Swim",
                    "SpikeRolling",
                    "Combat",
                    "CriticalHealth"
                };

                
                std::string unusedCumulCheck(filebuffer.data());
                for (int c = 5; c < 13; c++)
                {
                    if (!unusedCumulCheck.contains(playStates[c]))
                    {
                        mmrs_util::debug() << playStates[c] << " not in formmask, will be added as cumulative state..." << std::endl;
                        mmrs.formmask[16] += (1 << c);
                    }
                }

                int l = 0;
                while (std::getline(formmask, line, '"'))
                {
                    if (l++ % 2 == 0)
                    {
                        continue;
                    }
                    else if (!(line.contains("[") || line.contains("]")))
                    {
                        // mmrs_util::debug() << lineIdx << "(" << l << ") : " << line << std::endl;
                        if (lineIdx > 16)
                        {
                            mmrs_util::error() << "Formmask for " << zip_filename << " has more than 16 rows " << "(" << lineIdx << "), skipping!";
                            return false;
                        }

                        int stateIdx = 0;

                        if (line.contains("None"))
                        {
                            stateIdx = 5;
                        }
                        else if (line.contains("All"))
                        {
                            mmrs.formmask[lineIdx] += FIERCE_DEITY | GORON | ZORA | DEKU | HUMAN;
                            stateIdx = 5;
                        }

                        for (stateIdx; stateIdx < 12; stateIdx++)
                        {
                            if (line.contains(playStates[stateIdx]))
                            {
                                mmrs.formmask[lineIdx] += (1 << stateIdx);
                            }
                        }

                        lineIdx++;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else if (filename == "categories.txt") 
            {
                filebuffer.push_back('\0');
                char* c = std::strtok(filebuffer.data(), ",");

                while (c != nullptr) 
                {
                    int cat = 0;
                    try
                    {
                        cat = std::stoi(std::string(c), 0, 16);
                        // mmrs_util::debug() << std::string(c) << " : " << cat << std::endl;
                        if (cat < 256) 
                        {
                            mmrs.categories[cat] = true;
                            // mmrs_util::debug() << "Set category " << cat << " to " << mmrs.categories[cat] << ". " << std::endl;
                        }   
                        c = std::strtok(nullptr, ",");
                    }
                    catch(const std::exception& e)
                    {
                        mmrs_util::error() << "Could not parse int for category " << c << " in MMRS " << mmrs.songName << ". Song will be skipped." << std:: endl;
                        return false;
                    }
                }

                c = std::strtok(filebuffer.data(), "-");

                while (c != nullptr) 
                {
                    int cat = 0;
                    try
                    {
                        int cat = std::stoi(std::string(c), 0, 16);
                        // mmrs_util::debug() << std::string(c) << " : " << cat << std::endl;
                        if (cat < 256) 
                        {
                            mmrs.categories[cat] = true;
                        }
                        c = std::strtok(nullptr, "-");
                    }
                    catch(const std::exception& e)
                    {
                        mmrs_util::error() << "Could not parse int for category " << c << " in MMRS " << mmrs.songName << ". Song will be skipped." << std:: endl;
                        return false;
                    }
                    
                }
            }
            else if (filename.ends_with(".zbank")) 
            {
                found_zbank = true;

                zbankBuffer.resize(filesize);

                if (filesize > MAX_ZBANK_SIZE)
                {
                    throw std::runtime_error("Zbank file is too large - max 32 KiB!");
                }

                for (int j = 0; j < filesize; j++) 
                {
                    zbankBuffer[j] = (unsigned char)filebuffer[j];
                }
            }
            else if (filename.ends_with(".bankmeta")) 
            {
                found_bankmeta = true;

                bankmetaBuffer.resize(filesize);

                if (filesize > MAX_ZBANK_SIZE)
                {
                    throw std::runtime_error("File is too large - max 32 KiB!");
                }

                for (int j = 0; j < filebuffer.size(); j++) 
                {
                    if (j >= filebuffer.size()) break;
                    bankmetaBuffer[j] = (unsigned char)filebuffer[j];
                }
            }
            else if (filename.ends_with(".zsound"))
            {
                // mmrs_util::debug() << "Reading zsound file" << std::endl;

                Zsound zsound;
                zsound.size = filesize;
                std::string foreignKey = filename.substr(filename.length() - 15);
                
                try
                {
                    zsound.sampleAddr = std::stoi(foreignKey, 0, 16);
                }
                catch(const std::exception& e)
                {
                    mmrs_util::error() << "Could not parse int for zsound " << filename << " in MMRS " << mmrs.songName << ". Song will be skipped." << std:: endl;
                    return false;
                }
                


                for (int j = 0; j < MAX_ZSOUND_SIZE; j++)
                {
                    zsound.data[j] = 0;
                }

                // Make sure the extraction really succeeded.
                // mmrs_util::debug() << "Data was: ";
                // for (int j = 0; j < 16; j++) 
                // {
                //     if (j >= filebuffer.size()) break;
                //     mmrs_util::debug() << std::format("{:02x} ", filebuffer[j]);
                // }
                // mmrs_util::debug() << "...\n";

                if (filesize > MAX_ZSOUND_SIZE)
                {
                    throw std::runtime_error("Zsound file is too large - max 128 KiB");
                }

                for (int j = 0; j < filesize; j++) 
                {
                    zsound.data[j] = (unsigned char)filebuffer[j];
                }

                zsounds.push_back(zsound);

                // mmrs_util::debug() << "Successfully read!\n";
            }
            else 
            {
                mmrs_util::debug() << "Unknown filetype " << filename << std::endl;
            }
        }

        success = true;
        // Update the database
        // mmrs_util::debug() << "Categories: ";
        // for (int c = 0; c < 256; c++)
        // {
        //     if (mmrs.categories[c])
        //     {
        //         mmrs_util::debug() << c << " ";
        //     }
        // }
        // mmrs_util::debug() << std::endl;
        int mmrsId = insert_mmrs(mmrs, zseq, file);

        if (zsounds.size() > 0)
        {
            for (int s = 0; s < zsounds.size(); s++)
            {
                // mmrs_util::debug() << "SampleAddr is " << std::hex << zsounds[s].sampleAddr << std::endl;
                insert_zsound(zsounds[s], mmrsId);
            }
        }
        if (found_zbank && found_bankmeta)
        {
            Zbank zbank;
            zbank.metaSize = bankmetaBuffer.size();
            zbank.bankSize = zbankBuffer.size();
            // Initialize to 255 ("End of data")
            for (int j = 0; j < MAX_ZBANK_SIZE; j++)
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
        std::cerr << "MMRS Read error for song " << file.path().filename().string() << ": " << e.what() << "\n";
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
        mmrs_util::error() << "mz_error: " << mz_zip_get_error_string(mz_error) << "\n";
    }

    // Cleanup
    mz_zip_reader_end(&mz_archive);

    // Return true or false
    return success;
}

int read_seq_directory(const char* dbPath)
{
    const fs::path dir = "music";

    int status = 0;

    char *sqlErrorMsg;
    int sqlErrCode;
    std::unordered_set<std::string> filenames;

    if(fs::exists(dir))
    {
        if (fs::is_empty(dir))
        {
            return 0;
        }
        int i = 0;
        for(const fs::directory_entry entry: fs::directory_iterator(dir)) {

            std::string filename = entry.path().filename().string();
            std::string songNameStr = entry.path().filename().stem().string();
            const char* songName = songNameStr.c_str();

            // If file has an extension other than .mmrs, print that to the console and continue
            if (entry.path().extension() != ".mmrs") 
            {
                mmrs_util::info() << "File " << filename.c_str() << " is not a .mmrs file, skipping." << std::endl;
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
                    mmrs_util::debug() << "Reading file " << fs::absolute(entry.path()) << std::endl;

                    bool success = read_mmrs(entry);

                    if (!success) 
                    {
                        mmrs_util::error() << "Could not read file " << entry.path().filename().string() << "." << std::endl;
                        continue;
                    }

                    mmrs_util::debug() << "Successfully read file " << entry.path().filename().string() << std::endl;
                }
                i++;
            }
        }

        // mmrs_util::debug() << std::endl;
        // for (auto& it: filenames)
        // {
        //     mmrs_util::debug() << it << std::endl;
        // }

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
        mmrs_util::error() << std::endl << "Created path " << dir.string() << std::endl;
        fs::create_directory("music");
        return 0;
    }

    return -1;
}

RECOMP_DLL_FUNC(set_log_level)
{
    int level = RECOMP_ARG(int, 0);

    mmrs_util::set_log_level(mmrs_util::log_level_t(level));

    switch (level)
    {
        case mmrs_util::LOG_NOTHING:
            std::cout << "[MUSIC RANDOMIZER] Set log level to LOG_NOTHING." << std:: endl;
            break;
        case mmrs_util::LOG_CRITICAL:
            mmrs_util::critical() << "[MUSIC RANDOMIZER] Set log level to LOG_CRITICAL." << std:: endl;
            break;
        case mmrs_util::LOG_ERROR:
            mmrs_util::critical() << "[MUSIC RANDOMIZER] Set log level to LOG_ERROR." << std:: endl;
            break;
        case mmrs_util::LOG_WARNING:
            mmrs_util::critical() << "[MUSIC RANDOMIZER] Set log level to LOG_WARNING." << std:: endl;
            break;
        case mmrs_util::LOG_INFO:
            mmrs_util::critical() << "[MUSIC RANDOMIZER] Set log level to LOG_INFO." << std:: endl;
            break;
        case mmrs_util::LOG_DEBUG:
            mmrs_util::critical() << "[MUSIC RANDOMIZER] Set log level to LOG_DEBUG." << std:: endl;
            break;
    }

    RECOMP_RETURN(int, level);
}

RECOMP_DLL_FUNC(sql_init)
{
    std::string dbPathStr = RECOMP_ARG_STR(0);
    const char* dbPath = dbPathStr.c_str();
    mmrs_util::debug() << "init" << std::endl;

    if(_sql_init(dbPath))
    {
        mmrs_util::debug() << "Initialized SQL database successfully." << std::endl;
        RECOMP_RETURN(bool, true);
    }
    else
    {
        mmrs_util::error() << "Error initializing sql database: " << sqlite3_errmsg(db) << std::endl;
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
    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "START EXTLIB\n";

    std::string dbPathStr = RECOMP_ARG_STR(0);
    const char *dbPath = dbPathStr.c_str();
    
    bool initDb = false;
    try
    {
        initDb = init_mmrs_cache();
    }
    catch (std::exception e)
    {

        mmrs_util::error() << "Error initalizing music DB: " << e.what() << std::endl;
        RECOMP_RETURN(int, -1);
    }
    
    int numMmrs = read_seq_directory(dbPath);

    mmrs_util::debug() << "Completed extlib function read_mmrs_files(). Number of MMRSes: " << numMmrs;
    mmrs_util::debug() << END_PARA;
    

    RECOMP_RETURN(int, numMmrs);
}

RECOMP_DLL_FUNC(count_zsound)
{
    int mmrsId = RECOMP_ARG(int, 0);

    RECOMP_RETURN(int, _count_zsound(mmrsId));
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

    bool error = _load_mmrs_table(allMmrs);

    if(error)
    {
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

RECOMP_DLL_FUNC(load_zsounds)
{
    Zsound* zsoundAddr = RECOMP_ARG(Zsound*, 0);
    int mmrsId = RECOMP_ARG(int, 1);

    bool success = _load_zsound(zsoundAddr, mmrsId);
    
    if(success)
    {
        RECOMP_RETURN(bool, true);
    }
    else
    {
        RECOMP_RETURN(bool, false);
    }
}

RECOMP_DLL_FUNC(zsound_key_add)
{
    u32 key = RECOMP_ARG(u32, 0);
    u32 value = RECOMP_ARG(u32, 1);

    zsound_key_to_pointer[key] = value;

    // mmrs_util::debug() << "Added map entry " << std::format("{:02x} ", key) << " : " << std::format("{:02x} ", value) << std::endl;
}

RECOMP_DLL_FUNC(zsound_key_remove)
{
    u32 key = RECOMP_ARG(u32, 0);

    zsound_key_to_pointer.erase(key);
}

RECOMP_DLL_FUNC(zsound_key_lookup)
{
    u32 key = RECOMP_ARG(u32, 0);

    if (zsound_key_to_pointer.contains(key))
    {
        u32 value = zsound_key_to_pointer.at(key);
        
        RECOMP_RETURN(u32, value);
    }
    else
    {
        RECOMP_RETURN(u32, 0);
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