// C++ Libraries
#include "read_mmrs.h"
#include <stdexcept>
#include <fstream>

#define my_max(a, b) (((a) > (b)) ? (a) : (b))
#define my_min(a, b) (((a) < (b)) ? (a) : (b))

// #include "lib_recomp.hpp"

extern "C" {
    DLLEXPORT uint32_t recomp_api_version = 1;
}

void read_mmrs(fs::path in_path, MMRS* mmrs, uint8_t* rdram)
{
    printf("\n\nCalled read_mmrs() on file %s.\n", in_path.string().c_str());
    int fsize = fs::file_size(in_path);
    const char* mmrsPathCstr = in_path.string().c_str();
    
    mz_zip_archive mmrsArchive;

    printf("==READING FILE %s==\n", mmrsPathCstr);
    printf("MMRS at address %p\n", mmrs);

    memset(&mmrsArchive, 0, sizeof(mmrsArchive));
    const char* filename = in_path.filename().stem().string().c_str();
    for (int i = 0; i < strlen(filename); i++)
    {
        mmrs->songName[i ^ 3] = filename[i];
    }

    mz_bool status = mz_zip_reader_init_file(&mmrsArchive, mmrsPathCstr, 0);
    if (!status)
    {
        printf("Could not init file %s stats.\n", mmrsPathCstr);
        return;
    }
    else
    {
        printf("Successfully initiated file %s stats!\n", mmrsPathCstr);
    }
    printf("we ");

    unsigned char data[2] = {0xD3, 0x20};
    
    for (int i = 0; i < (int)mz_zip_reader_get_num_files(&mmrsArchive); i++)
    {
        mz_zip_archive_file_stat stat;
        status = mz_zip_reader_file_stat(&mmrsArchive, i, &stat);
        printf("are ", (int)mz_zip_reader_get_num_files(&mmrsArchive));
        
        if (!status) {
            printf("Could not read file %s stats.\n", stat.m_filename);
            return;
        }

        printf("SO ");
        size_t uncomp_size;

        void *file = mz_zip_reader_extract_file_to_heap(&mmrsArchive, stat.m_filename, &uncomp_size, 0);

        if (!file)
        {
            printf("mz_zip_reader_extract_file_to_heap() failed!\n");
            mz_zip_reader_end(&mmrsArchive);
            return;
        }

        printf("back.\n");
        std::string filename = stat.m_filename;
        int size = stat.m_comp_size;

        // Should be different if categories.txt
        if (filename.ends_with(".zseq") || filename.ends_with(".seq"))
        {
            printf("Reading %s from archive.\n", filename.c_str());

            // Make sure the extraction really succeeded.
            printf("Data was ");
            const char *grgha = (const char*)file;
            for (int q = 0; q < 16; q++)
            {
                printf("%hhx ", grgha[q]);
            }
            printf("...\n");

            if ((memcmp((const char*)file, data, 2)))
            {
                printf("mz_zip_reader_extract_file_to_heap() failed to extract the proper data\n");
                mz_free(file);
                mz_zip_reader_end(&mmrsArchive);
                throw;
            }

            if (stat.m_uncomp_size > 32768)
            {
                printf("Zseq file %s is too large - max 32 KiB.", filename.c_str());
                throw;
            }

            printf("Copying from file at address %p into zseq at address %p. Size %i\n", file, &mmrs->zseq.data[0], stat.m_uncomp_size);
            for (int j = 0; j < stat.m_uncomp_size; j++)
            {
                // memcpy(mmrs->zseq.data, (unsigned char*)file, sizeof(unsigned char) * stat.m_uncomp_size);
                mmrs->zseq.data[j ^ 3] = ((unsigned char*) file)[j];
            }

            printf("Successfully wrote!\n");
            mmrs->zseq.bankNo = std::stoi(filename, 0, 16);
            mmrs->zseq.size = stat.m_uncomp_size;

            printf("Zseq is at %p\n", mmrs->zseq);
            printf("Size is %i at addr %p\n", mmrs->zseq.size, &mmrs->zseq.size);
            printf("Data starts with %hhx\n", mmrs->zseq.data[0]);
        }

        else if (filename == "categories.txt")
        {
            printf("Reading categories.txt.\n");
            std::string categories = (std::string)(char*)file;
            int cat;

            char *c = std::strtok(categories.data(), ",");

            while (c != nullptr)
            {
                cat = std::stoi(std::string(c));
                mmrs->categories[cat] = true;
                c = std::strtok(nullptr, ",");
            }
        }
        else
        {
            printf("Found file %s\n", filename.c_str());
        }
    }
}

extern "C" int read_seq_directory(MMRS* mmrsTable, uint8_t* rdram) 
{

    printf("\nQwerty and/or uiop\n");
    const fs::path dir = "music";

    int status = 0;

    if(fs::exists(dir)) 
    {
        int i = 0;
        for(const fs::directory_entry entry: fs::directory_iterator(dir))
        {
            // If file has an extension other than .mmrs, print that to the console and continue
            if (entry.path().extension() != ".mmrs")
            {
                printf("File %s is not a .mmrs file!", entry.path().filename().string().c_str());
            }
            // Otherwise, read it as an mmrs
            else
            {
                std::cout << fs::absolute(entry.path()) << std::endl;
                try {
                    read_mmrs(entry.path(), &mmrsTable[i], rdram);

                } catch (const std:: exception &e)
                {
                    printf("Could not read file %s.", entry.path().filename().string().c_str());
                    continue;
                }

                printf("Successfully read file %s\n", entry.path().filename().string().c_str());
                printf("Zseq pointer is %p\n", &mmrsTable[i].zseq);
                printf("Size is %i\n\n", mmrsTable[i].zseq.size);
            }
            printf("i: %d\n\n", i);
            i++;
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
        for(const fs::directory_entry entry: fs::directory_iterator(dir))
        {
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

    MMRS *mmrsTable = RECOMP_ARG(MMRS*, 0);
    printf("Zseq address of element 0 before mutandum: %p\n", mmrsTable[0].zseq);
    printf("MMRS table address before call: %p\n", mmrsTable);
    printf("Zseq address of element 0 before call: %p\n", mmrsTable[0].zseq);

    int numMmrs = read_seq_directory(mmrsTable, rdram);
    printf("\nwoag !! it made it through the thing??!?!?!?!?\n");
    printf("MMRS table address now: %p\n\n", mmrsTable);
    printf("Example zseq data at entry 1 (little endian): ");
    for (int q = 0; q < 16; q++)
    {
        printf("%02x ", mmrsTable[1].zseq.data[q]);
    }
    printf("...\n");
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