#include "database.h"

Database::Database(fs::path dbPath)
{
    const char* path = dbPath.c_str();

    int rc = sqlite3_open(path, &db);

    lastRC = rc;
    lastErrMsg = (char*)sqlite3_errmsg(db);

    init_tables();
}

Database* Database::database_ = nullptr;

Database* Database::get_db(fs::path path)
{
    if (database_ == nullptr)
    {
        database_ = new Database(path);
    }
    else if (database_->path != path)
    {
        // Add error here
        return nullptr;
    }
    else
    {
        return database_;
    }
}

Database* Database::get_db()
{
    if (database_ == nullptr)
    {
        database_ = new Database((fs::path)"mod_data\\music");
    }

    return database_;
}

bool Database::init_tables()
{
    TrackTable* tracks = new TrackTable(this);
    SequenceTable* seqs = new SequenceTable(this);
    BankTable* banks = new BankTable(this);
    SoundTable* sounds = new SoundTable(this);

    RelationTable* track_to_seq = new RelationTable(this, "track_to_seq", "trackId", "seqId");
    RelationTable* track_to_bank = new RelationTable(this, "track_to_seq", "trackId", "bankId");
    RelationTable* track_to_sound = new RelationTable(this, "track_to_seq", "trackId", "soundId");
}

int Table::exec(std::string query)
{
    return db->exec(query);
}

bool Table::init(std::string query)
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

    auto ([&]
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