// C++ Libraries
#include "read_mmrs.h"
#include <stdexcept>


MMRS read_mmrs(fs::path in_file)
{
    lz::ZipArchive mmrs_archive(in_file);
    lz::ZipEntry file;
    int err;

    mmrs_archive.setErrorHandlerCallback([](const std::string& message,
                                const std::string& strerror,
                                int zip_error_code,
                                int system_error_code)
        {
            // Handle error here
            fprintf(stderr, message.c_str(), strerror.c_str());
        });

    // If error, throw error
    if (!mmrs_archive.open(lz::ZipArchive::ReadOnly)) 
    {
        throw std::runtime_error("Could not read file.");
    }
    // otherwise, create the necessary structs
    else
    {
        fs::path title = in_file.stem();

        MMRS mmrs;
        mmrs.songName = std::string(title).data();

        std::vector<lz::ZipEntry> entries = mmrs_archive.getEntries();

        for(std::vector<lz::ZipEntry>::iterator it=entries.begin() ; it!=entries.end(); ++it)
        {
            file = *it;
            
            std::string filename = file.getName();
            int size = file.getSize();

            // Should be different if categories.txt
            if (filename.ends_with(".zseq"))
            {
                Zseq *song = new Zseq;
                song->data = file.readAsBinary();
                song->bankNo = std::stoi(file.getName(), 0, 16);
                song->size = file.getSize();

                mmrs.zseq = song;
            }

            if (filename == "categories.txt")
            {
                std::string categories = file.readAsText();
                int cat;

                char *c = std::strtok(categories.data(), ",");

                while (c != nullptr)
                {
                    cat = std::stoi(std::string(c));
                    mmrs.categories[cat] = true;
                    c = std::strtok(nullptr, ",");
                }
            }
        }

        return mmrs;
    }
}

extern "C" MMRS* read_seq_directory() 
{
    const fs::path dir = "mmrs";
    std::vector<MMRS> allMmrs;

    int status = 0;

    if(fs::exists(dir)) 
    {
        for(const fs::directory_entry entry: fs::directory_iterator(dir))
        {
            // If file has an extension other than .mmrs, print that to the console and continue
            if (entry.path().extension() != ".mmrs")
            {
                std::cout << "File " << entry.path().filename() << " is not a .mmrs file!" << std::endl;
            }
            // Otherwise, read it as an mmrs
            else
            {
                MMRS mmrs;
                std::cout << fs::absolute(entry.path()) << std::endl;
                try {
                    mmrs = read_mmrs(entry.path());

                } catch (const std:: runtime_error &e)
                {
                    std::cerr << "Could not read file " << entry.path().filename() << "." << std::endl;
                    return nullptr;
                }

                std::cout << "Successfully read file " << entry.path().filename() << "!" << std::endl;
                allMmrs.push_back(mmrs);

            }
        }
    }
    else
    {
        MMRS *returnArray = new MMRS[allMmrs.size()];
        std::copy(allMmrs.begin(), allMmrs.end(), returnArray);
        return returnArray;
    }

    MMRS *returnArray = new MMRS[allMmrs.size()];
        std::copy(allMmrs.begin(), allMmrs.end(), returnArray);
        return returnArray;
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