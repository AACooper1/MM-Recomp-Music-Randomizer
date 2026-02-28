#include "audiofile.h"

void AudioFile::read_from_file(std::shared_ptr<std::vector<char>> file)
{
    size = file->size();
    data = file;
}

bool Sound::parse_foreignKey()
{
    try
    {
        std::string foreignKey = filename.substr(filename.length() - 15);
        sampleAddr = std::stoi(foreignKey, 0, 16);
        return true;
    }
    catch (const std::exception& e)
    {
        logger.error << "Could not parse zsound " << filename << ", skipping!\n";
        return false;
    }
}

void Sequence::read_into_mod_memory(void* modAddr) {}
void Sound::read_into_mod_memory(void* modAddr) {}
void Bank::read_into_mod_memory(void* modAddr) {}
void Stream::read_into_mod_memory(void* modAddr) {}

void Sequence::read_from_database(int index) {}
void Bank::read_from_database(int id) {}
void Sound::read_from_database(int id) {}
void Stream::read_from_database(int id) {}