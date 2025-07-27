#define SQL_ERR_CHECK(errMsg, okMsg)                                                 \
    if (rc != SQLITE_OK)                                                             \
    {                                                                                \
        mmrs_util::error() << errMsg << ": " << sqlite3_errmsg(db) << std::endl;     \
        mmrs_util::error() << "Error code is " << rc << std::endl;                   \
        sqlite3_close(db);                                                           \
        return false;                                                                \
    }                                                                                \
    else                                                                             \
    {                                                                                \
        mmrs_util::info() << okMsg << std::endl;                                     \
    }                                                                                \

bool init_mmrs_cache(const char* dbPath);

bool check_mmrs_exists(fs::directory_entry file);

bool insert_mmrs(MMRS mmrs, fs::directory_entry file);