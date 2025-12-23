#ifndef READ_MUSIC_H
#define READ_MUSIC_H

#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#include "miniz.h"
#include "database.h"

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
        virtual void read_into_database(std::vector<char> file) = 0;
        virtual void read_from_database(int id) = 0;

        virtual void read_into_mod_memory(void* modAddr) = 0;

        int get_database_id() { return databaseIndex; }
    protected:
        Database* db;
        AudioFileType type;

        unsigned char* data;
        int size;

        int databaseIndex;
};

class Sequence : public AudioFile
{
    public:
        void read_into_database(std::vector<char> file) override;
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
};

class Bank : public AudioFile
{
    public:
        void read_into_database(std::vector<char> file) override;
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
    private:
        unsigned char header[8];
};

class Sound : public AudioFile
{
    public:
        void read_into_database(std::vector<char> file) override;
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
    private:
        u32 sampleAddr;
};

class Stream : public AudioFile
{
    public:
        void read_into_database(std::vector<char> file) override;
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
};

class AudioFileFactory
{
    public:
        std::unique_ptr<AudioFile> read_file(AudioFileType type)
        {
            switch (type)
            {
                case AudioFileType::ZSEQ: return std::make_unique<Sequence>();
                case AudioFileType::ZBANK: return std::make_unique<Bank>();
                case AudioFileType::ZSOUND: return std::make_unique<Sound>();
                case AudioFileType::STREAMED: return std::make_unique<Stream>();
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