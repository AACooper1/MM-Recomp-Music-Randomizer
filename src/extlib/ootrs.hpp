#ifndef OOTRS_H
#define OOTRS_H

#include <filesystem>
#include <vector>
#include <unordered_map>
#include <span>
#include <bit>

#include "miniz.h"

#include "lib_recomp.hpp"

#include "logging.hpp"
#include "songslot.h"
#include "sha1.hpp"

#define AUDIOTABLE_HEADER "Audiotable_index"
#define AUDIOTABLE "Audiotable"
#define BANKTABLE_HEADER "Audiobank_index"
#define BANKTABLE "Audiobank"

// As defined by MM RomDesc
#define MM_BANKTABLE_HEADER_OFFSET 0xC776C0
#define MM_BANKTABLE_HEADER_LENGTH 0x2A0
#define MM_BANKTABLE_OFFSET 0x20700
#define MM_BANKTABLE_LENGTH 0x263F0

#define MM_AUDIOTABLE_HEADER_OFFSET 0xC78380
#define MM_AUDIOTABLE_HEADER_LENGTH 0x40
#define MM_AUDIOTABLE_OFFSET 0x97F70
#define MM_AUDIOTABLE_LENGTH 0x548770

typedef std::span<u8> ByteArray;

typedef enum SampleBankTableType {
    /* 0 */ SEQUENCE_TABLE,
    /* 1 */ FONT_TABLE,
    /* 2 */ SAMPLE_TABLE
} SampleBankTableType;

typedef struct AudioTableHeader {
    /* 0x00 */ s16 numEntries;
    /* 0x02 */ s16 unkMediumParam;
    /* 0x04 */ uintptr_t romAddr;
    /* 0x08 */ char pad[0x8];
} AudioTableHeader; // size = 0x10

typedef struct AudioTableEntry {
    /* 0x0 */ uintptr_t romAddr;
    /* 0x4 */ size_t size;
    /* 0x8 */ s8 medium;
    /* 0x9 */ s8 cachePolicy;
    /* 0xA */ s16 shortData1;
    /* 0xC */ s16 shortData2;
    /* 0xE */ s16 shortData3;
} AudioTableEntry; // size = 0x10

typedef struct AudioTable {
    /* 0x00 */ AudioTableHeader header;
    /* 0x10 */ AudioTableEntry *entries; // (dynamic size)
} AudioTable; // size >= 0x20

inline short int16_from_bytes(ByteArray bytes, short idx)
{
    short ret = 0;
    std::memcpy(&ret, bytes.data() + idx, 2);
    return std::endian::native == std::endian::little ? std::byteswap(ret) : ret;
}

inline int int32_from_bytes(ByteArray bytes, int idx)
{
    int ret = 0;
    std::memcpy(&ret, bytes.data() + idx, 4);
    ret = std::endian::native == std::endian::little ? std::byteswap(ret) : ret;
    return ret;
}

enum class SampleType
{
    DRUM,
    SFX,
    INST_LOW,
    INST_MED,
    INST_HIGH
};

// Adapted from OoTR's Music.py, credit to rrealmuto https://github.com/OoTRandomizer/OoT-Randomizer/blob/Dev/Audiobank.py
struct Sample
{
    SampleType type;
    int idx;

    ByteArray sample_header;
    int size = 0;
    int sampleAddr = 0;
    int audiotableId = 0;
    int audiotableAddr = 0;
    int sample_offset;
    ByteArray data;
    
    Sample(ByteArray bankdata, ByteArray audiotable, ByteArray audiotable_header, int sample_offset, int audiotable_id, SampleType type, int idx)
    {
        this->type = type;
        this->idx = idx;
        
        this->sample_header = bankdata.subspan(sample_offset, 0x10);
        this->sample_offset = sample_offset;
        int bitfield = std::byteswap(*(int*)sample_header.data());
        this->size = bitfield & 0xFFFFFF;
        this->sampleAddr = int32_from_bytes(sample_header, 4);
        this->audiotableId = audiotable_id;

        // Mark as Zsound
        if (this->sampleAddr > audiotable.size())
        {
            this->sampleAddr = -1;
        }
        else
        {
            int audiotable_index_offset = 0x10 + (audiotable_id * 0x10); // This is the samplebank id I think
            ByteArray audiotable_entry = audiotable_header.subspan(audiotable_index_offset, 0x10);
            int audiotable_offset = int32_from_bytes(audiotable_entry, 0);
            int sample_idx = audiotable_offset + this->sampleAddr;
            this->audiotableAddr = sample_idx;

            this->data = audiotable.subspan(this->audiotableAddr, this->size);
        }
    }
    Sample() 
    {
        this->sampleAddr = 0;
    };
};

struct Drum
{
    int drum_id = 0;
    int release_rate = 0;
    int pan = 0;
    int sampleOffset = 0;
    int sampleTuning = 0;
    int envelopePointOffset = 0;
    Sample sample;

    Drum(int drum_id, ByteArray bankdata, ByteArray audiotable, ByteArray audiotable_header, int drum_offset, int audiotable_id)
    {
        this->drum_id = drum_id;
        this->release_rate = bankdata[drum_offset];
        this->pan = bankdata[drum_offset + 1];
        this->sampleOffset = int32_from_bytes(bankdata, drum_offset + 4);
        this->sampleTuning = int32_from_bytes(bankdata, drum_offset + 8);
        this->envelopePointOffset = int32_from_bytes(bankdata, drum_offset + 12);
        this->sample = Sample(bankdata, audiotable, audiotable_header, this->sampleOffset, audiotable_id, SampleType::DRUM, drum_id);
    }
};

struct SFX
{
    int sfx_id = 0;
    int sampleOffset = 0;
    int sampleTuning = 0;
    Sample sample;

    SFX(int sfx_id, ByteArray bankdata, ByteArray audiotable, ByteArray audiotable_header, int sfx_offset, int audiotable_id)
    {
        this->sfx_id = sfx_id;
        this->sampleOffset = int32_from_bytes(bankdata, sfx_offset);
        this->sampleTuning = int32_from_bytes(bankdata, sfx_offset + 4);
        this->sample = Sample(bankdata, audiotable, audiotable_header, this->sampleOffset, audiotable_id, SampleType::SFX, sfx_id);
    }
};

struct Instrument
{
    int inst_id = 0;
    int normalRangeLo = 0;
    int normalrangeHi = 0;
    int releaseRate = 0;
    int AdsrEnvelopePointOffset = 0;
    int lowNoteSampleOffset = 0;
    int lowNoteTuning = 0;
    int normalNoteSampleOffset = 0;
    int normalNoteTuning = 0;
    int highNoteSampleOffset = 0;
    int highNoteTuning = 0;
    Sample lowNoteSample;
    Sample normalNoteSample;
    Sample highNoteSample;

    Instrument(int inst_id, ByteArray bankdata, ByteArray audiotable, ByteArray audiotable_header, int inst_offset, int audiotable_id)
    {
        this->inst_id = inst_id;
        this->normalRangeLo = bankdata[inst_offset + 1];
        this->normalrangeHi = bankdata[inst_offset + 2];
        this->releaseRate = bankdata[inst_offset + 3];
        this->AdsrEnvelopePointOffset = int32_from_bytes(bankdata, inst_offset + 4);
        this->lowNoteSampleOffset = int32_from_bytes(bankdata, inst_offset + 8);
        this->lowNoteTuning = int32_from_bytes(bankdata, inst_offset + 12);
        this->normalNoteSampleOffset = int32_from_bytes(bankdata, inst_offset + 16);
        this->normalNoteTuning = int32_from_bytes(bankdata, inst_offset + 20);
        this->highNoteSampleOffset = int32_from_bytes(bankdata, inst_offset + 24);
        this->highNoteTuning = int32_from_bytes(bankdata, inst_offset + 28);

        this->lowNoteSample = lowNoteSampleOffset ? Sample(bankdata, audiotable, audiotable_header, this->lowNoteSampleOffset, audiotable_id, SampleType::INST_LOW, inst_id) : Sample();
        this->normalNoteSample = normalNoteSampleOffset ? Sample(bankdata, audiotable, audiotable_header, this->normalNoteSampleOffset, audiotable_id, SampleType::INST_MED, inst_id) : Sample();
        this->highNoteSample = highNoteSampleOffset ? Sample(bankdata, audiotable, audiotable_header, this->highNoteSampleOffset, audiotable_id, SampleType::INST_HIGH, inst_id) : Sample();
    }
};

struct AudioBank
{
    int bankNo;
    int bank_offset; // Offset of the bank in the Audiobank file
    int size; // Size of the bank, in bytes
    int medium; // ROM/RAM/DISK
    int type;
    int audiotable_id; // Samplebank number
    int unk; // Unk still got it
    int num_instruments;
    int num_drums;
    int num_sfx;
    
    ByteArray bank_data;
    std::vector<u8> original_data;
    ByteArray table_entry;
    std::vector<int> duplicate_banks;

    int drum_offset;
    std::vector<Drum> drums;
    int sfx_offset;
    std::vector<SFX> sfx;
    int inst_offset;
    std::vector<Instrument> instruments;

    std::vector<Sample*> zsounds_to_add;

    void read_stuff(ByteArray audiotable, ByteArray audiotable_header)
    {
        // Read drums
        this->drum_offset = int32_from_bytes(this->bank_data, 0);
        for (int i = 0; i < this->num_drums; i++)
        {
            int offset = drum_offset + 4 * i;
            offset = int32_from_bytes(this->bank_data, offset);
            if (!offset) { continue; }
            Drum drum = Drum(i, this->bank_data, audiotable, audiotable_header, offset, this->audiotable_id);
            this->drums.push_back(drum);
        }

        // Read SFX
        this->sfx_offset = int32_from_bytes(this->bank_data, 4);
        for (int i = 0; i < this->num_sfx; i++)
        {
            int offset = sfx_offset + 8 * i;
            // offset = int32_from_bytes(this->bank_data, offset);
            if (!offset) { continue; }
            SFX sfx = SFX(i, this->bank_data, audiotable, audiotable_header, offset, this->audiotable_id);
            this->sfx.push_back(sfx);
        }

        // Read instruments
        this->inst_offset = 8;
        for (int i = 0; i < this->num_instruments; i++)
        {
            int offset = inst_offset + 4 * i;
            offset = int32_from_bytes(this->bank_data, offset); // Why is this not a pointer to pointers like drums are?? This doesn't make any sense
            if (!offset) { continue; }
            Instrument instrument = Instrument(i, this->bank_data, audiotable, audiotable_header, offset, this->audiotable_id);
            this->instruments.push_back(instrument);
        }
    }

    AudioBank(int bankNo, ByteArray table_entry, ByteArray audiobank, ByteArray audiotable, ByteArray audiotable_header)
    {
        this->bankNo = bankNo;
        this->bank_offset = int32_from_bytes(table_entry, 0);
        this->size = int32_from_bytes(table_entry, 4);
        this->medium = table_entry[8];
        this->type = table_entry[9];
        this->audiotable_id = table_entry[10];
        this->unk = table_entry[11];
        this->num_instruments = table_entry[12];
        this->num_drums = table_entry[13];
        this->num_sfx = int16_from_bytes(table_entry, 14);
        
        this->bank_data = audiobank.subspan(this->bank_offset, size);
        this->original_data.assign(this->bank_data.begin(), this->bank_data.end());
        this->table_entry = table_entry;
        this->duplicate_banks = std::vector<int>(); // Not necessary but it's there for the sake of completeness

        read_stuff(audiotable, audiotable_header);
    }

    AudioBank(int bankNo, int size, ByteArray table_entry, ByteArray audiobank, ByteArray audiotable, ByteArray audiotable_header)
    {
        this->bankNo = bankNo;
        this->bank_offset = 0;
        this->size = size;
        this->medium = table_entry[0];
        this->type = table_entry[1];
        this->audiotable_id = table_entry[2];
        this->unk = table_entry[3];
        this->num_instruments = table_entry[4];
        this->num_drums = table_entry[5];
        this->num_sfx = int16_from_bytes(table_entry, 6);
        
        this->bank_data = audiobank.subspan(this->bank_offset, size);
        this->original_data.assign(this->bank_data.begin(), this->bank_data.end());
        this->table_entry = table_entry;
        this->duplicate_banks = std::vector<int>(); // Not necessary but it's there for the sake of completeness

        read_stuff(audiotable, audiotable_header);
    }

    std::string str_info()
    {
        std::stringstream ret;
        ret << "Bank ID: " << this->bankNo << "\n" << 
               "Offset:  " << this->bank_offset << "\n" <<
               "Length:  " << this->size << "\n";

        return ret.str();
    }

    std::vector<Sample*> get_all_samples()
    {

        std::vector<Sample*> all_samples;

        for (int i = 0; i < this->drums.size(); i++)
        {
            if(drums[i].sample.sampleAddr != -1)
            {
                all_samples.push_back(&drums[i].sample);
            }
        }
        for (int i = 0; i < this->sfx.size(); i++)
        {
            if(sfx[i].sample.sampleAddr != -1)
            {
                all_samples.push_back(&sfx[i].sample);
            }
        }
        for (int i = 0; i < this->instruments.size(); i++)
        {
            if(instruments[i].lowNoteSample.sampleAddr > 0)
            {
                all_samples.push_back(&instruments[i].lowNoteSample);
            }
            if(instruments[i].normalNoteSample.sampleAddr > 0)
            {
                all_samples.push_back(&instruments[i].normalNoteSample);
            }
            if(instruments[i].highNoteSample.sampleAddr > 0)
            {
                all_samples.push_back(&instruments[i].highNoteSample);
            }
        }

        return all_samples;
    }

    std::vector<u8> build_entry(int offset)
    {
        std::vector<u8> bank_entry = std::vector<u8>(16);
        std::memcpy(bank_entry.data(), &offset, 4);
        int banksize = this->bank_data.size();
        std::memcpy(bank_entry.data() + 4, &banksize, 4);
        std::memcpy(bank_entry.data() + 8, this->table_entry.data() + 8, 8);
        
        return bank_entry;
    }
};


class OoTAudioHandler
{
    public:
        OoTAudioHandler(fs::path path);
        void prepare_oot_audio();
        AudioBank match_custom_oot_bank(std::shared_ptr<std::vector<u8>> header, std::shared_ptr<std::vector<u8>> data);
        std::vector<std::vector<u8>> ootZsounds;

        int numOoTBanks = 0;
        std::vector<AudioBank> ootBanks;

        bool successfully_parsed = false;

    private:
        bool unzip_oot_audiobin();
        bool copy_mm_rom();
        std::vector<u8> decompress_rom(std::span<const uint8_t> compressed_rom);
        bool get_mm_files();
        bool get_all_banks();
        void get_all_mm_samples();

        int find_sample_in_mm_banks(ByteArray sample_data, int audioTableId);
        bool match_all_oot_banks();

        mz_zip_archive archive;

        std::unordered_map<std::string, ByteArray> ootFiles;
        std::unordered_map<std::string, ByteArray> mmFiles;

        fs::path audiobinPath;
        std::string expected_files[4] = { AUDIOTABLE_HEADER, AUDIOTABLE, BANKTABLE_HEADER, BANKTABLE};
        std::vector<std::vector<u8>> ootFilesRaw;

        std::vector<Sample*> all_mm_samples;

        fs::path mmRomPath;
        std::vector<u8> mmRomRaw;
        ByteArray mmRom;
        int numMMBanks = 0;
        std::vector<AudioBank> mmBanks;
};

#endif