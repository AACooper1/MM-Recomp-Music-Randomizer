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

Bank::Bank(AudioBank& bank)
{
    std::shared_ptr<std::vector<char>> bankData = std::make_shared<std::vector<char>>();
    bankData->assign(bank.bank_data.begin(), bank.bank_data.end());
    
    std::shared_ptr<std::vector<char>> bankHeader = std::make_shared<std::vector<char>>();
    bankHeader->assign(bank.table_entry.begin() + 8, bank.table_entry.end());

    this->header = bankHeader;
    this->data = bankData;
    this->size = bankData->size();
}

Sound::Sound(Sample* sound)
{
    std::shared_ptr<std::vector<char>> soundData = std::make_shared<std::vector<char>>();
    soundData->assign(sound->data.begin(), sound->data.end());
    this->data = soundData;
    this->sampleAddr = sound->sampleAddr;
    this->size = soundData->size();
}