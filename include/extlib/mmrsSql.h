#define SQL_ERR_CHECK(errMsg, okMsg)                                                 \
    if (rc != SQLITE_OK)                                                             \
    {                                                                                \
        mmrs_util::error() << errMsg << ": " << sqlite3_errmsg(db) << std::endl;     \
        mmrs_util::error() << "Error code is " << rc << std::endl;                   \
        return false;                                                                \
    }                                                                                \
    else                                                                             \
    {                                                                                \
        mmrs_util::info() << okMsg << std::endl;                                     \
    }                                                                                \

int count_mmrs();

bool init_mmrs_cache(const char* dbPath);

bool check_mmrs_exists(fs::directory_entry file);

bool insert_mmrs(MMRS mmrs, Zseq zseq, fs::directory_entry file);

bool _load_mmrs_table(const char *dbPath, MMRS* allMmrs);

bool _load_zseq(const char *dbPath, Zseq* zseqAddr, int zseqId);

bool retrieve_filenames(int* ids, std::string* filenames);

bool remove_mmrs(int mmrsId);

bool sql_teardown();