// C++ Libraries
#include "read_mmrs.h"
#include <stdexcept>
#include <format>
#include <fstream>
#include <iostream>

#define my_max(a, b) (((a) > (b)) ? (a) : (b))
#define my_min(a, b) (((a) < (b)) ? (a) : (b))

// #include "lib_recomp.hpp"

extern "C" 
{
    DLLEXPORT uint32_t recomp_api_version = 1;
}

bool read_mmrs(fs::path in_path, MMRS* mmrs, uint8_t* rdram)

{
    // mz vars, defined here so we have access outside the try block
    mz_zip_archive mz_archive;
    mz_zip_error mz_error;
    mz_bool mz_status;

    // Did this function succeed?
    bool success;

    try 
    {
        int zip_filesize = fs::file_size(in_path);
        std::string zip_filename = in_path.filename().stem().string();

        std::cout << "\n================================================================================\n";
        std::cout << "Called read_mmrs() on file " << zip_filename;
        std::cout << "\n================================================================================\n";

        std::cout << "File size: " << zip_filesize << "\n";
        std::cout << "MMRS at address: " << &mmrs << "\n";

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
        std::cout << "Successfully opened ZIP with " << num_files << " files!\n";

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
            std::cout << "\nReading file " <<  filename << " with size " << filesize << "\n";

            mz_status = mz_zip_reader_extract_to_mem(&mz_archive, i, filebuffer.data(), filebuffer.size(), 0);
            if (!mz_status) 
            {
                throw std::runtime_error("mz_zip_reader_extract_to_mem() failed");
            }

            // Should be different if categories.txt
            if (filename.ends_with(".zseq") || filename.ends_with(".seq")) 
            {
                std::cout << "Reading sequence file\n";

                // Make sure the extraction really succeeded.
                std::cout << "Data was: ";
                for (int j = 0; j < 16; j++) 
                {
                    if (j >= filebuffer.size()) break;
                    printf("%hhx ", filebuffer[j]);
                }
                std::cout << "...\n";

                if ((unsigned char)filebuffer.at(0) != 0xD3 || (unsigned char)filebuffer.at(1) != 0x20) 
                {
                    throw std::runtime_error("Invalid zseq header");
                }

                if (filesize > 32768) 
                {
                    throw std::runtime_error("File is too large - max 32 KiB");
                }

                std::cout << "Copying from file at address " << &filebuffer;
                std::cout << " into zseq at address " << &mmrs->zseq.data[0] << "\n";

                for (int j = 0; j < filesize; j++) 
                {
                    // memcpy(mmrs->zseq.data, (unsigned char*)file, sizeof(unsigned char) * stat.m_uncomp_size);
                    mmrs->zseq.data[j ^ 3] = (unsigned char)filebuffer[j];
                }

                std::cout << "Successfully wrote!\n";

                mmrs->zseq.bankNo = std::stoi(filename, 0, 16);
                mmrs->zseq.size = filesize;

                std::cout << "Zseq is at " << &mmrs->zseq << "\n";
                std::cout << "Size is " << mmrs->zseq.size << " at " << &mmrs->zseq.size << "\n";

                printf("Data starts with %hhx\n", mmrs->zseq.data[0]);
            }
            else if (filename == "categories.txt") {
                std::cout << "Reading categories.txt file\n";
                std::cout << "Categories: ";

                char* c = std::strtok(filebuffer.data(), ",");
                while (c != nullptr) 
                {
                    int cat = std::stoi(std::string(c));
                    if (cat < 256) 
                    {
                        mmrs->categories[cat] = true;
                        std::cout << cat << " ";
                    }
                    c = std::strtok(nullptr, ",");
                }
                std::cout << "\n";
            }
            else 
            {
                std::cout << "Unknown filetype\n";
            }
        }

        success = true;

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
        std::cout << "mz_error: " << mz_zip_get_error_string(mz_error) << "\n";
    }

    std::cout << "================================================================================\n\n";

    // Cleanup
    mz_zip_reader_end(&mz_archive);

    // Return true or false
    return success;
}

extern "C" int read_seq_directory(MMRS* mmrsTable, uint8_t* rdram)
{

    printf("\nQwerty and/or uiop\n");
    const fs::path dir = "music";

    int status = 0;

    if(fs::exists(dir)) 
    {
        int i = 0;
        for(const fs::directory_entry entry: fs::directory_iterator(dir)) {
            printf("i: %d\n\n", i);

            // If file has an extension other than .mmrs, print that to the console and continue
            if (entry.path().extension() != ".mmrs") 
            {
                printf("File %s is not a .mmrs file!", entry.path().filename().string().c_str());
            }
            // Otherwise, read it as an mmrs
            else 
            {
                std::cout << fs::absolute(entry.path()) << std::endl;

                bool success = read_mmrs(entry.path(), &mmrsTable[i], rdram);

                if (!success) 
                {
                    printf("Could not read file %s.", entry.path().filename().string().c_str());
                    continue;
                }

                printf("Successfully read file %s\n", entry.path().filename().string().c_str());
                printf("Zseq pointer is %p\n", &mmrsTable[i].zseq);
                printf("Size is %i\n\n", mmrsTable[i].zseq.size);

                i++;
            }
        }
    } 
    else 
    {
        printf("\ndir %s\\%s does not exist.\n", fs::current_path().string().c_str(), dir.string().c_str());
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

    std::cout << "\n================================================================================\n";
    std::cout << "START EXTLIB";
    std::cout << "\n================================================================================\n";

    MMRS *mmrsTable = RECOMP_ARG(MMRS*, 0);
    printf("Zseq address of element 0 before mutandum: %p\n", &mmrsTable[0].zseq);
    printf("MMRS table address before call: %p\n", mmrsTable);
    printf("Zseq address of element 0 before call: %p\n", &mmrsTable[0].zseq);

    int numMmrs = read_seq_directory(mmrsTable, rdram);
    printf("\nwoag !! it made it through the thing??!?!?!?!?\n");
    printf("MMRS table address now: %p\n\n", mmrsTable);
    printf("Example zseq data at entry 1 (little endian): ");
    
    for (int q = 0; q < 16; q++) 
    {
        printf("%02x ", mmrsTable[1].zseq.data[q]);
    }

    std::cout << "\n================================================================================\n";
    std::cout << "END EXTLIB";
    std::cout << "\n================================================================================\n";
}


// #ifdef __cplusplus
// int main()
// {
//     std::cout << fs::current_path() << std::endl;
//     MMRS* allMmrs;
//     allMmrs = read_seq_directory();

//     return 0;
// }
// #endif
