#ifndef AUDIOFILE_H
#define AUDIOFILE_H

#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

#include "miniz.h"
#include "logging.h"

#define MAX_ZSEQ_SIZE 32768     // 32 KiB
#define MAX_ZBANK_SIZE 32768    // 32 KiB
#define MAX_ZSOUND_SIZE 131072  // 128 KiB
#define MAX_MMRS_SIZE 1048576   // 1 MiB

#ifndef u32 
    typedef uint32_t u32;
#endif

namespace fs = std::filesystem;
extern Log logger;

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
        void read_from_file(std::shared_ptr<std::vector<char>> file);
        virtual void read_from_database(int id) = 0;

        virtual void read_into_mod_memory(void* modAddr) = 0;

        int databaseIndex = 0;
        int audioTableIndex = 0;
        std::shared_ptr<std::vector<char>> data;
        int size;

        AudioFileType getType() { return type; }
    protected:
        AudioFile(std::shared_ptr<std::vector<char>> filebuffer, std::string filename) : 
            _fb(filebuffer), 
            filename(filename) 
            { read_from_file(_fb); }
        AudioFileType type;

        std::shared_ptr<std::vector<char>> _fb;
        std::string filename;
};

class Sequence : public AudioFile
{
    public:
        Sequence(std::shared_ptr<std::vector<char>> filebuffer, std::string filename) : AudioFile(filebuffer, filename)
        {
            type = AudioFileType::ZSEQ; 
        };
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
};

class Bank : public AudioFile
{
    public:
        Bank(std::shared_ptr<std::vector<char>> filebuffer, std::string filename, bool is_bankmeta) : AudioFile(filebuffer, filename)
        {
            if (is_bankmeta) { header = filebuffer; }
            type = AudioFileType::ZBANK;
        }
        void read_header(std::shared_ptr<std::vector<char>> filebuffer) { header = filebuffer; };
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;

        std::shared_ptr<std::vector<char>> header;
};

class Sound : public AudioFile
{
    public:
        Sound(std::shared_ptr<std::vector<char>> filebuffer, std::string filename) : AudioFile(filebuffer, filename)
        {
            type = AudioFileType::ZSOUND;
        }
        bool parse_foreignKey();
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
        
        u32 sampleAddr;
};

class Stream : public AudioFile
{
    public:
        Stream(std::shared_ptr<std::vector<char>> filebuffer, std::string filename) : AudioFile(filebuffer, filename)
        {
            type = AudioFileType::STREAMED;
        }
        void read_from_database(int id) override;

        void read_into_mod_memory(void* modAddr) override;
};

#endif