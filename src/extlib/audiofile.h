#ifndef READ_MUSIC_H
#define READ_MUSIC_H

#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#include "miniz.h"
#include "database.h"
#include "audio/load.h"

#define MAX_ZSEQ_SIZE 32768     // 32 KiB
#define MAX_ZBANK_SIZE 32768    // 32 KiB
#define MAX_ZSOUND_SIZE 131072  // 128 KiB
#define MAX_MMRS_SIZE 1048576   // 1 MiB

#ifndef u32 
    typedef uint32_t u32;
#endif

namespace fs = std::filesystem;

enum class AudioFileType
{
    ZSEQ,
    ZBANK,
    ZSOUND,
    STREAMED
};

class AudioFile
{
    public:
        void read_from_file(std::vector<char> file);
        virtual void read_from_database(int id) = 0;

        virtual void read_into_mod_memory(void* modAddr) = 0;

        int get_database_id() { return databaseIndex; }
    protected:
        Database* db = nullptr;
        AudioFileType type;

        char* data;
        int size;

        int databaseIndex = 0;
};

class Sequence : public AudioFile
{
    public:
        Sequence(std::vector<char> file) 
        {
            type = AudioFileType::ZSEQ; 
            read_from_file(file);
        };
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
};

class Bank : public AudioFile
{
    public:
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
    private:
        unsigned char header[8];
};

class Sound : public AudioFile
{
    public:
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
    private:
        u32 sampleAddr;
};

class Stream : public AudioFile
{
    public:
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
};



class AudioFileFactory
{
    public:
        AudioFile* read_file(std::string filename, std::vector<char> file)
        {
            if (filename.ends_with(".zseq") || filename.ends_with(".seq"))
                return new Sequence(file);
            if (filename.ends_with(".zbank"))
                return new Bank(file);
            if (filename.ends_with(".zsound"))
                return new Sound(file);
            if (filename.ends_with(".mp3") || filename.ends_with(".ogg") || filename.ends_with("wav"))
                return new Stream(file);

            else
            {
                return nullptr;
            }
        }
};

class FormMask
{
    public:
        unsigned short state;
};

extern "C"
{
    struct CustomSong
    {
        
    };
}

#endif