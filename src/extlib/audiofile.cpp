#include "audiofile.h"

void AudioFile::read_from_file(std::vector<char> file)
{
    size = file.size();
    data = file.data();
}

void Sequence::read_from_database(int index)
{
    TrackTable* table;
}

void Sequence::read_into_mod_memory(void* modAddr) {}
void Sound::read_into_mod_memory(void* modAddr) {}
void Bank::read_into_mod_memory(void* modAddr) {}
void Stream::read_into_mod_memory(void* modAddr) {}

void Sequence::read_from_database(int id) {}
void Bank::read_from_database(int id) {}
void Sound::read_from_database(int id) {}
void Stream::read_from_database(int id) {}

void Sequence::read_into_mod_memory(void* modAddr) {}
void Bank::read_into_mod_memory(void* modAddr) {}
void Sound::read_into_mod_memory(void* modAddr) {}
void Stream::read_into_mod_memory(void* modAddr) {}