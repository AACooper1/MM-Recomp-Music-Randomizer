// Project files
#include "read_mmrs.h"
#include "mmrsSql.h"

// C/C++ std Libraries
#include <stdexcept>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

// C/C++ external libraries

#define my_max(a, b) (((a) > (b)) ? (a) : (b))
#define my_min(a, b) (((a) < (b)) ? (a) : (b))

sqlite3 *db;

extern "C" 
{
    DLLEXPORT uint32_t recomp_api_version = 1;
}

bool read_mmrs(fs::directory_entry file, MMRS* mmrs, uint8_t* rdram)
{
    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Calling read_mmrs on file " << file.path().string() << "!";
    mmrs_util::debug() << END_PARA;

    // mz vars, defined here so we have access outside the try block
    mz_zip_archive mz_archive;
    mz_zip_error mz_error;
    mz_bool mz_status;

    // Did this function succeed?
    bool success;

    fs::path in_path = file.path();

    try 
    {
        int zip_filesize = fs::file_size(in_path);
        std::string zip_filename = in_path.filename().stem().string();

        mmrs_util::debug() << "File size: " << zip_filesize << "\n";
        mmrs_util::debug() << "MMRS at address: " << &mmrs << "\n";

        // Write filename to mod memory
        for (int i = 0; i < zip_filename.length(); i++) 
        {
            mmrs->songName[i ^ 3] = zip_filename[i];
        }

        // Clear the mz_zip_archive struct then try to read file
        memset(&mz_archive, 0, sizeof(mz_archive));

        // Try to read the ZIP file
        mz_status = mz_zip_reader_init_file(&mz_archive, in_path.string().c_str(), 0);
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
            size_t filesize = (size_t)stat.m_uncomp_size;
            std::vector<char> filebuffer(filesize);

            std::string filename = stat.m_filename;
            mmrs_util::debug() << "\nReading file " <<  filename << " with size " << filesize << "\n";

            mz_status = mz_zip_reader_extract_to_mem(&mz_archive, i, filebuffer.data(), filebuffer.size(), 0);
            if (!mz_status) 
            {
                throw std::runtime_error("mz_zip_reader_extract_to_mem() failed");
            }

            // Should be different if categories.txt
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

                if (filesize > 32768) 
                {
                    throw std::runtime_error("File is too large - max 32 KiB");
                }

                mmrs_util::debug() << "Copying from file at address " << &filebuffer;
                mmrs_util::debug() << " into zseq at address " << &mmrs->zseq.data[0] << "\n";

                for (int j = 0; j < filesize; j++) 
                {
                    // memcpy(mmrs->zseq.data, (unsigned char*)file, sizeof(unsigned char) * stat.m_uncomp_size);
                    mmrs->zseq.data[j ^ 3] = (unsigned char)filebuffer[j];
                }

                mmrs_util::debug() << "Successfully wrote!\n";

                mmrs->zseq.bankNo = std::stoi(filename, 0, 16);
                mmrs->zseq.size = filesize;

                mmrs_util::debug() << "Zseq is at " << &mmrs->zseq << "\n";
                mmrs_util::debug() << "Size is " << mmrs->zseq.size << " at " << &mmrs->zseq.size << "\n";

                printf("Data starts with %hhx\n", mmrs->zseq.data[0]);
            }
            else if (filename == "categories.txt") {
                mmrs_util::debug() << "Reading categories.txt file\n";
                mmrs_util::debug() << "Categories: ";

                char* c = std::strtok(filebuffer.data(), ",");
                while (c != nullptr) 
                {
                    int cat = std::stoi(std::string(c));
                    if (cat < 256) 
                    {
                        mmrs->categories[cat] = true;
                        mmrs_util::debug() << cat << " ";
                    }
                    c = std::strtok(nullptr, ",");
                }
                mmrs_util::debug() << "\n";
            }
            else 
            {
                mmrs_util::debug() << "Unknown filetype " << filename << std::endl;
            }
        }

        success = true;
        // Update the database
        insert_mmrs(*mmrs, file);

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

extern "C" int read_seq_directory(MMRS* mmrsTable, uint8_t* rdram, const char* dbPath)
{
    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Calling read_seq_directory!";
    mmrs_util::debug() << END_PARA;

    int rc = sqlite3_open(dbPath, &db);

    SQL_ERR_CHECK("Error connecting to MMRS database", "Successfully connected to MMRS database!");

    const fs::path dir = "music";

    int status = 0;

    char *sqlErrorMsg;
    int sqlErrCode;

    if(fs::exists(dir)) 
    {
        int i = 0;
        for(const fs::directory_entry entry: fs::directory_iterator(dir)) {
            printf("i: %d\n\n", i);

            std::string filename = entry.path().filename().string();
            const char* songName = entry.path().filename().stem().string().c_str();

            // If file has an extension other than .mmrs, print that to the console and continue
            if (entry.path().extension() != ".mmrs") 
            {
                printf("File %s is not a .mmrs file, skipping.", filename.c_str());
            }
            else 
            {
                if (check_mmrs_exists(entry))
                {
                    continue;
                }
                else 
                {
                    mmrs_util::debug() << fs::absolute(entry.path()) << std::endl;

                    bool success = read_mmrs(entry, &mmrsTable[i], rdram);

                    if (!success) 
                    {
                        mmrs_util::error() << "Could not read file " << entry.path().filename().string();
                        continue;
                    }

                    mmrs_util::debug() << "Successfully read file " << entry.path().filename().string() << std::endl;
                    mmrs_util::debug() << "Zseq pointer is" << &mmrsTable[i].zseq << std::endl;
                    mmrs_util::debug() << "Size is " << mmrsTable[i].zseq.size << std::endl;

                    i++;
                }
            }
        }
    } 
    else 
    {
        mmrs_util::error() << "\ndir" << fs::current_path().string() << "\\" << dir.string() << "does not exist.\n";
        return -2;
    }

    return 0;
}

RECOMP_DLL_FUNC(get_num_mmrs)
{
    int ct = 0;
    const fs::path dir = "music";

    if(fs::exists(dir)) 
    {
        for(const fs::directory_entry entry: fs::directory_iterator(dir)) {
            // If file has an extension other than .mmrs, print that to the console and continue
            if (entry.path().extension() == ".mmrs") 
            {
                ct++;
            }
        }
    }

    RECOMP_RETURN(int, ct);
}

RECOMP_DLL_FUNC(read_mmrs_files)
{
    mmrs_util::set_log_level(mmrs_util::LOG_DEBUG);
    
    if (mmrs_util::gLogLevel >= mmrs_util::LOG_DEBUG)
    {
        printf(START_PARA);
        printf("START EXTLIB");
        printf(END_PARA);
    }

    MMRS *mmrsTable = RECOMP_ARG(MMRS*, 0);

    bool initDb = false;
    const char *dbPath = "assets/musicDB.db";

    if (mmrs_util::gLogLevel >= mmrs_util::LOG_DEBUG) { printf("Calling init_mmrs_cache!");}
    try
    {
        initDb = init_mmrs_cache(dbPath);
    }
    catch (std::exception e)
    {
        if (mmrs_util::gLogLevel >= mmrs_util::LOG_ERROR) 
        { 
            printf("Error initalizing music DB: %s\n", e.what());
            return;
        }
    }

    // if(mmrs_util::gLogLevel >= mmrs_util::LOG_DEBUG)
    // {
        printf("\nBefore read_seq_directory, MMRS table was:\n");
        sqlite3_stmt *statement;
        const char* query = "SELECT * FROM mmrs;";

        int rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr);

        while ((rc = sqlite3_step(statement)) == SQLITE_ROW)
        {
            printf("%s, timestamp %lli\n", sqlite3_column_text(statement, 1), sqlite3_column_int64(statement, 2));
        }
        if (rc != SQLITE_ROW)
        {
            printf("Returned code %i", rc);
        }
    // }
    // printf("\nwoag !! it made it through the thing??!?!?!?!?\n");            PRESERVED FOR FUTURE GENERATIONS
    

    int numMmrs = read_seq_directory(mmrsTable, rdram, dbPath);

    printf("\nAfter read_seq_directory, MMRS table was:\n");
        query = "SELECT * FROM mmrs;";

        rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr);

        while ((rc = sqlite3_step(statement)) == SQLITE_ROW)
        {
            printf("%s, timestamp %lli\n", sqlite3_column_text(statement, 1), sqlite3_column_int64(statement, 2));
        }
        if (rc != SQLITE_ROW)
        {
            printf("Returned code %i", rc);
        }

    sqlite3_close(db);

    for (int q = 0; q < 16; q++) 
    {
        if (mmrs_util::gLogLevel >= mmrs_util::LOG_DEBUG){   printf("%02X", mmrsTable[1].zseq.data[q]);}
    }

    if (mmrs_util::gLogLevel >= mmrs_util::LOG_DEBUG)
    {
        printf(START_PARA);
        printf("END EXTLIB");
        printf(END_PARA);
    }
}