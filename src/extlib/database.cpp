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
    this->tables = std::make_unique<dbTables>();
}

Database::~Database() 
{ 
    sqlite3_close(db.get()); 
}

void Database::init()
{
    try
        {
            init_tables();
        }
        catch (std::exception& e)
        {
            logger.error << "Could not initialize database: " << e.what() << std::endl;
            throw e;
        }
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

template <typename ...T>
void Database::bind(sqlite3_stmt* statement, std::string query, T&&... args)
{
    int i = 1;

    ([&]
    {
             if constexpr (typeid(args) == typeid(int32_t))     { sqlite3_bind_int(statement, i, args); }
        else if constexpr (typeid(args) == typeid(int64_t))     { sqlite3_bind_int64(statement, i, args); }
        else if constexpr (typeid(args) == typeid(std::string)) { sqlite3_bind_text(statement, i, args.c_str(), args.length(), SQLITE_STATIC);}
        else if constexpr (typeid(args) == typeid(bool*))       { sqlite3_bind_blob(statement, i, args, sizeof(args) * 0x200, SQLITE_STATIC); }
        else                                                    { sqlite3_bind_blob(statement, i, &args, sizeof(args), SQLITE_STATIC); }
        i++;
    } (), ...
    );
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

// I think it's easier if insert() just does upsert by default
template<> int TrackTable::insert(std::unique_ptr<Track> entry) 
{
    sqlite3_stmt* statement;

    std::string query = 
        "INSERT INTO track (             \
            filename,                   \
            modified,                   \
            songName,                   \
            categories,                 \
            bankNo,                     \
            formMask                    \
        )                               \
        VALUES (?, ?, ?, ?, ?, ?)       \
        ON CONFLICT (filename) DO       \
        UPDATE SET                      \
            modified=?,                 \
            songName=?,                 \
            categories=?,               \
            bankNo=?,                   \
            formMask=?                  \
        RETURNING id;";

    if (sqlite3_prepare_v2(get_sqlite().get(), query.c_str(), -1, &statement, nullptr))
    {
        return -2;
    }

    db->bind(statement, query, 
        entry->path.string(),
        entry->timestamp, 
        entry->name,
        entry->categories,
        entry->bankNo,
        entry->formmask,

        entry->timestamp,
        entry->name,
        entry->categories,
        entry->bankNo,
        entry->formmask
    );

    if ((rc = sqlite3_step(statement)) == SQLITE_ROW)
    {
        entry->databaseIndex = sqlite3_column_int(statement, 0);
    }

    sqlite3_finalize(statement);

    return 0;
}

template<typename T> int Table<T>::insert(std::unique_ptr<T> entry) { return false; }

template<> bool TrackTable::remove(int id) {return false;}
template<> bool TrackTable::remove(std::string query) {return false;}
template <typename T> bool Table<T>::remove(int id) {return false;}
template <typename T> bool Table<T>::remove(std::string query) {return false;}

template<> bool Table<Track>::update(std::unique_ptr<Track> entry) { return false; }
template<typename T> bool Table<T>::update(std::unique_ptr<T> entry) { return false; }

template<> std::shared_ptr<Track> TrackTable::select(std::string query) {}
template<> std::shared_ptr<Track> TrackTable::select(std::string query, std::string* cols) {}
template<> std::shared_ptr<Track> TrackTable::select(int id) {}

template<typename T> std::shared_ptr<T> Table<T>::select(std::string query) {}
template<typename T> std::shared_ptr<T> Table<T>::select(std::string query, std::string* cols) {}
template<typename T> std::shared_ptr<T> Table<T>::select(int id) {}


bool TrackToSequenceTable::remove(int id) {return false;}
bool TrackToBankTable::remove(int id) {return false;}
bool TrackToSoundTable::remove(int id) {return false;}

bool TrackToSequenceTable::insert(int id_1, int id_2) {return false;}
bool TrackToBankTable::insert(int id_1, int id_2) {return false;}
bool TrackToSoundTable::insert(int id_1, int id_2) {return false;}

int TrackToSequenceTable::select(int id) {return false;}
int TrackToBankTable::select(int id) {return false;}
int TrackToSoundTable::select(int id) {return false;}