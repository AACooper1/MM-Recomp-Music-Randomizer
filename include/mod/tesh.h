typedef struct Zseq_t {
    int size;
    void* data;
    int bankNo;
    bool formmask[16];
} Zseq;


typedef struct Zbank_t{
    int bankSize;
    void* bankData;
    int MetaSize;
    void* MetaData;
} Zbank;


typedef struct Zsound_t {
    char *size;
    void* data;
} Zsound;


typedef struct MMRSFileType {
    char* songName;
    bool categories[256];
    Zseq *zseq;
    Zbank *bankInfo;
    Zsound *soundInfo;
} MMRS;