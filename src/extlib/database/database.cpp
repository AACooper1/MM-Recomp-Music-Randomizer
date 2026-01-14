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
        std::shared_ptr<Track> track = std::make_shared<Track>(entry.path());
        if (track->type == TrackType::UNKNOWN)
        {
            continue;
        }

        add_track(track);
    }

    return 0;
}

bool Database::add_track(std::shared_ptr<Track>& track)
{
    if (track->read_from_file()) 
    {
        int trackNo = tables->track->insert(track);

        if (track->type != TrackType::STREAMED)
        {
            int seqNo = tables->seq->insert(track->sequence);
            tables->relation.track_to_seq->insert(trackNo, seqNo);
        }
        if (track->bank)
        {
            int bankNo = tables->bank->insert(track->bank);
            tables->relation.track_to_bank->insert(trackNo, bankNo);
        }
        for (int i = 0; i < track->sounds.size(); i++) 
            tables->sound->insert(track->sounds[i]);
        
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

template<typename T> int Table<T>::insert(std::shared_ptr<T> entry) { return false; }

template<> bool TrackTable::remove(int id) {return false;}
template<> bool TrackTable::remove(std::string query) {return false;}
template <typename T> bool Table<T>::remove(int id) {return false;}
template <typename T> bool Table<T>::remove(std::string query) {return false;}

template<> bool Table<Track>::update(std::shared_ptr<Track> entry) { return false; }
template<typename T> bool Table<T>::update(std::shared_ptr<T> entry) { return false; }

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