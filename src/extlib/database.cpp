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
    
    std::string dbPath_forSql = dbPath.string();

    lastErrMsg = new char;

    sqlite3* dbPtrRaw = nullptr;

    int rc = sqlite3_open(dbPath_forSql.c_str(), &dbPtrRaw);

    set_last_rc(rc);

    this->db.reset(dbPtrRaw, sqlite3_close);

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
    else {errMsg << "SQL Error (" << lastRC << "): " << sqlite3_errmsg(this->db.get());}
    std::string error = errMsg.str();
    throw std::runtime_error(error);
}

int Database::exec(std::string query)
{
    const char* query_forSqlite = query.c_str();
    int rc = sqlite3_exec(this->sqlite().get(), query_forSqlite, nullptr, nullptr, &lastErrMsg);

    return rc;
}

void Database::init_tables()
{
    tables->track = std::make_unique<TrackTable>(shared_from_this());
    tables->seq = std::make_unique<SequenceTable>(shared_from_this());
    tables->bank = std::make_unique<BankTable>(shared_from_this());
    tables->sound = std::make_unique<SoundTable>(shared_from_this());

    tables->relation.track_to_seq = std::make_unique<TrackToSequenceTable>(shared_from_this(), "track_to_seq");
    tables->relation.track_to_bank = std::make_unique<TrackToBankTable>(shared_from_this(), "track_to_bank");
    tables->relation.track_to_sound = std::make_unique<TrackToSoundTable>(shared_from_this(), "track_to_sound");
}

int Database::update_from_music_dir()
{
    if (!fs::exists(musicPath))
    {
        return 2;
    }

    for(const fs::directory_entry entry: fs::recursive_directory_iterator(musicPath)) 
    {
        std::unique_ptr<Track> track = std::make_unique<Track>(entry.path());
        if (track->type == TrackType::UNKNOWN)
        {
            continue;
        }

        add_track(track);
    }

    return 0;
}

bool Database::add_track(std::unique_ptr<Track>& track)
{
    if (track->read_from_file()) 
    {
        tables->track->insert(std::move(track));
        return true;
    }
    else return false;
}


template <typename T>
int Table<T>::exec(std::string query)
{
    return this->db->exec(query);
}

RelationTable::RelationTable(std::shared_ptr<Database> db, std::string name)
{
    this->db = db;

    std::string query = std::format(
        "CREATE TABLE IF NOT EXISTS %s ( "
            "%s INTEGER PRIMARY KEY,     ",
            "%s INTEGER                  ",
        name, col1, col2
    );

    db->exec(query);
}

bool RelationTable::remove(int id) {}


template<> TrackTable::Table(std::shared_ptr<Database> db) : db(db)
{
    std::string query =         
        "CREATE TABLE IF NOT EXISTS track (          \
            id INTEGER PRIMARY KEY AUTOINCREMENT,    \
            filename TEXT UNIQUE,                    \
            modified INTEGER,                        \
            songName TEXT,                           \
            categories BLOB,                         \
            bankNo INTEGER,                          \
            formMask BLOB                            \
        );";

    db->exec(query);
}

template<> SequenceTable::Table(std::shared_ptr<Database> db) : db(db)
{
    std::string query =
        "CREATE TABLE IF NOT EXISTS seq (             "
            "id INTEGER PRIMARY KEY AUTOINCREMENT,    "
            "size INTEGER,                            "
            "data BLOB                                "
        ");";

    db->exec(query);
}

template<> BankTable::Table(std::shared_ptr<Database> db): db(db)
{
    std::string query =
        "CREATE TABLE IF NOT EXISTS bank (                   "
            "id INTEGER PRIMARY KEY AUTOINCREMENT,           "
            "headerSize INTEGER,                             "
            "header BLOB,                                    "
            "dataSize INTEGER,                               "
            "data BLOB                                       "
        ");";

    db->exec(query);
}

template<> SoundTable::Table(std::shared_ptr<Database> db): db(db)
{
    std::string query =
            "CREATE TABLE IF NOT EXISTS sound (                 "
            "id INTEGER PRIMARY KEY AUTOINCREMENT,           "
            "size INTEGER,                                   "
            "foreignKey INTEGER,                             "
            "data BLOB                                       "
        ");";

    db->exec(query);
}

template<> bool TrackTable::remove(int id) {return false;}
template<> bool TrackTable::remove(std::string query) {return false;}
template <typename T> bool Table<T>::remove(int id) {return false;}
template <typename T> bool Table<T>::remove(std::string query) {return false;}


// I think it's easier if insert() just does upsert by default
template<> bool TrackTable::insert(std::unique_ptr<Track> entry) { return false; }
template<> bool SequenceTable::insert(std::unique_ptr<Sequence> entry) { return false; }
template<> bool BankTable::insert(std::unique_ptr<Bank> entry) { return false; }
template<> bool SoundTable::insert(std::unique_ptr<Sound> entry) { return false; }

template<> bool Table<Track>::update(std::unique_ptr<Track> entry) { return false; }
template<> bool SequenceTable::update(std::unique_ptr<Sequence> entry) { return false; }
template<> bool BankTable::update(std::unique_ptr<Bank> entry) { return false; }
template<> bool SoundTable::update(std::unique_ptr<Sound> entry) { return false; }

template<> std::shared_ptr<Track> TrackTable::select(std::string query) {}
template<> std::shared_ptr<Track> TrackTable::select(std::string query, std::string* cols) {}
template<> std::shared_ptr<Track> TrackTable::select(int id) {}

template<> std::shared_ptr<Sequence> SequenceTable::select(std::string query) {}
template<> std::shared_ptr<Sequence> SequenceTable::select(std::string query, std::string* cols) {}
template<> std::shared_ptr<Sequence> SequenceTable::select(int id) {}
template<> std::shared_ptr<Bank> BankTable::select(std::string query) {}
template<> std::shared_ptr<Bank> BankTable::select(std::string query, std::string* cols) {}
template<> std::shared_ptr<Bank> BankTable::select(int id) {}
template<> std::shared_ptr<Sound> SoundTable::select(std::string query) {}
template<> std::shared_ptr<Sound> SoundTable::select(std::string query, std::string* cols) {}
template<> std::shared_ptr<Sound> SoundTable::select(int id) {}


bool TrackToSequenceTable::remove(int id) {return false;}
bool TrackToBankTable::remove(int id) {return false;}
bool TrackToSoundTable::remove(int id) {return false;}

bool TrackToSequenceTable::insert(int id_1, int id_2) {return false;}
bool TrackToBankTable::insert(int id_1, int id_2) {return false;}
bool TrackToSoundTable::insert(int id_1, int id_2) {return false;}

int TrackToSequenceTable::select(int id) {return false;}
int TrackToBankTable::select(int id) {return false;}
int TrackToSoundTable::select(int id) {return false;}