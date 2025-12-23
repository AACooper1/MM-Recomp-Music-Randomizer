#include "database.h"

Database::Database(fs::path path)
{
    logger.dev << "Entering Database constructor here" << std::endl;
    this->dbPath = path;
    this->musicPath = dbPath / "music";

    if (!fs::exists(this->musicPath))
    {
        fs::create_directories(this->musicPath);
        logger.info << "Created new directory " << musicPath << "." << std::endl;
    }

    bool initDb = false;

    this->dbPath /= "music.db";
    const char* dbPath_forSql = dbPath.string().c_str();
    lastErrMsg = new char;

    int rc = sqlite3_open(dbPath_forSql, &this->db);

    set_last_rc(rc);

    try
    {
        init_tables();
    }
    catch (std::exception e)
    {
        logger.error << "Could not initialize database: " << e.what() << std::endl;
        throw e;
    }
}

Database* Database::database_ = nullptr;

Database* Database::get_db(fs::path path)
{
    logger.dev << "Hello?" << std::endl;
    std::cout << "Testing" << std::endl;
    if (database_ == nullptr)
    {
        logger.dev << "Did not find database. Creating new one." << std::endl;
        database_ = new Database(path);
    }
    else if (database_->dbPath != path)
    {
        throw std::runtime_error("Specified database path did not match existing path!");
    }
    return database_;
}

Database* Database::get_db()
{
    if (database_ == nullptr)
    {
        throw std::runtime_error("Database path must be specified if not already initialized!");
    }

    return database_;
}

void Database::set_last_rc(int rc)
{
    switch (rc)
    {
        case SQLITE_OK:
        case SQLITE_DONE:
        case SQLITE_ROW:
        case SQLITE_ERROR:      // Sqlite returns a generic error if you try to create a duplicate table. Even if "IF NOT EXISTS" is there. I haven't seen this error anywhere else.
            lastRC = rc;
            break;
        default:
            lastRC = rc;
            report_error();
    }
}

void Database::report_error()
{
    if (*lastErrMsg != '\0') {errMsg  << "SQL Error (" << lastRC << "): " << lastErrMsg;}
    else {errMsg << "SQL Error (" << lastRC << "): " << sqlite3_errmsg(this->db);}
    std::string error = errMsg.str();
    throw std::runtime_error(error);
}

int Database::exec(std::string query)
{
    const char* query_forSqlite = query.c_str();
    int rc = sqlite3_exec(this->sqlite(), query_forSqlite, nullptr, nullptr, &lastErrMsg);

    return rc;
}

void Database::init_tables()
{
    TrackTable* tracks = new TrackTable(this);
    SequenceTable* seqs = new SequenceTable(this);
    BankTable* banks = new BankTable(this);
    SoundTable* sounds = new SoundTable(this);

    TrackToSequenceTable* track_to_seq = new TrackToSequenceTable(this, "track_to_seq", "trackId", "seqId");
    TrackToBankTable* track_to_bank = new TrackToBankTable(this, "track_to_seq", "trackId", "bankId");
    TrackToSoundTable* track_to_sound = new TrackToSoundTable(this, "track_to_sound", "trackId", "soundId");
}

void Database::update()
{
    if (!fs::exists(this->dbPath))
    {
        fs::create_directories(this->dbPath);
        logger.info << "Created new directory " << dbPath << "." << std::endl;
    }

    bool initDb = false;
}




int Table::exec(std::string query)
{
    return this->db->exec(query);
}

void Table::init(std::string query)
{
    const char* query_cstr = query.c_str();

    int rc = sqlite3_exec(
        get_sqlite(),
        query_cstr,
        nullptr,
        nullptr,
        &db->lastErrMsg
    );

    db->set_last_rc(rc);
}

TrackTable::TrackTable(Database* db)
{
     this->db = db;

     std::string query =         
        "CREATE TABLE IF NOT EXISTS track (         \
           id INTEGER PRIMARY KEY AUTOINCREMENT,    \
           filename TEXT UNIQUE,                    \
           modified INTEGER,                        \
           songName TEXT,                           \
           categories BLOB,                         \
           bankNo INTEGER,                          \
           formMask BLOB                            \
        );";

    init(query);

    this->db->tables.insert({"track", this});
}

SequenceTable::SequenceTable(Database* db)
{
    this->db = db;

    std::string query =
    "CREATE TABLE IF NOT EXISTS seq (             "
        "id INTEGER PRIMARY KEY AUTOINCREMENT,    "
        "size INTEGER,                            "
        "data BLOB                                "
    ");";

    init(query);
}

BankTable::BankTable(Database* db)
{
    this->db = db;

    std::string query =
    "CREATE TABLE IF NOT EXISTS bank (                   "
        "id INTEGER PRIMARY KEY AUTOINCREMENT,           "
        "headerSize INTEGER,                             "
        "header BLOB,                                    "
        "dataSize INTEGER,                               "
        "data BLOB                                       "
    ");";

    init(query);
}

SoundTable::SoundTable(Database* db)
{
    this->db = db;

    std::string query = 
    "CREATE TABLE IF NOT EXISTS sound (                  "
        "id INTEGER PRIMARY KEY AUTOINCREMENT,           "
        "size INTEGER,                                   "
        "foreignKey INTEGER,                             "
        "data BLOB                                       "
    ");";

    init(query);
}

RelationTable::RelationTable(Database* db, std::string name, std::string primaryKey, std::string cols...)
{
    this->db = db;

    std::string query = std::format(
        "CREATE TABLE IF NOT EXISTS %s ( "
            "%s INTEGER PRIMARY KEY,     ",
        name,
        primaryKey
    );

    void ([&]
    {
        query += std::format(
            " %s INTEGER,",
            cols
        );
    }
    );

    cols += "\b);";

    init(query);
}

Database::~Database()
{
    int rc = sqlite3_finalize(statement);
    rc = sqlite3_close(db);
}

bool TrackTable::remove(int id) {return false;}
bool SoundTable::remove(int id) {return false;}
bool SequenceTable::remove(int id) {return false;}
bool BankTable::remove(int id) {return false;}

bool TrackTable::remove(std::string query) {return false;}
bool SoundTable::remove(std::string query) {return false;}
bool SequenceTable::remove(std::string query) {return false;}
bool BankTable::remove(std::string query) {return false;}

AudioFile* SequenceTable::select(std::string query) {}
AudioFile* SequenceTable::select(std::string query, std::string* cols) {}
AudioFile* SequenceTable::select(int id) {}
AudioFile* BankTable::select(std::string query) {}
AudioFile* BankTable::select(std::string query, std::string* cols) {}
AudioFile* BankTable::select(int id) {}
AudioFile* SoundTable::select(std::string query) {}
AudioFile* SoundTable::select(std::string query, std::string* cols) {}
AudioFile* SoundTable::select(int id) {}

bool TrackToSequenceTable::remove(int id) {return false;}
bool TrackToBankTable::remove(int id) {return false;}
bool TrackToSoundTable::remove(int id) {return false;}

bool TrackToSequenceTable::remove(std::string query) {return false;}
bool TrackToBankTable::remove(std::string query) {return false;}
bool TrackToSoundTable::remove(std::string query) {return false;}

bool TrackToSequenceTable::insert(int id_1, int id_2) {return false;}
bool TrackToBankTable::insert(int id_1, int id_2) {return false;}
bool TrackToSoundTable::insert(int id_1, int id_2) {return false;}

int TrackToSequenceTable::select(int id) {return false;}
int TrackToBankTable::select(int id) {return false;}
int TrackToSoundTable::select(int id) {return false;}