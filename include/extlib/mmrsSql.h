// #ifndef SQL_ERR_CHECK
    #define SQL_ERR_CHECK(errMsg, okMsg)                                                 \
        if (rc != SQLITE_OK)                                                             \
        {                                                                                \
            mmrs_util::error() << errMsg << ": " << sqlite3_errmsg(db) << std::endl;     \
            mmrs_util::error() << "Error code is " << rc << std::endl;                   \
            return false;                                                                \
        }                                                                                \
        else                                                                             \
        {                                                                                \
            mmrs_util::debug() << okMsg;                                                 \
        }
// #endif

bool _sql_init(const char* dbPath);

int count_mmrs();

int _count_zsound(int mmrsId);

bool init_mmrs_cache();

bool check_mmrs_exists(fs::directory_entry file);

int insert_mmrs(MMRS mmrs, Zseq zseq, fs::directory_entry file);

int insert_zbank(Zbank zbank, int mmrsId);

bool insert_zsound(Zsound zsound, int mmrsId);

bool _load_mmrs_table(MMRS* allMmrs);

bool _load_zseq(Zseq* zseqAddr, int zseqId);

bool _load_zbank(Zbank* zbankAddr, int zbankId);

bool _load_zsound(Zsound* zsoundAddr, int zsoundId);

bool retrieve_filenames(int* ids, std::string* filenames);

bool remove_mmrs(int mmrsId);

bool _sql_teardown();