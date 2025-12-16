#include "audiofile.h"

void Sequence::read_into_database(std::vector<char> file)
{

    std::size_t filesize = file.size();
    if (filesize > MAX_ZSEQ_SIZE) throw std::runtime_error("Zseq file is too large - max 1 MiB!\n");

    for (int j = 0; j < filesize; j++) 
    {
        this->data[j] = (unsigned char)file[j];
    }
}

void Sequence::read_from_database(int index)
{
    TrackTable* table;
}