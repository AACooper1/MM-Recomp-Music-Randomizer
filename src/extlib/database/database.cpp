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

bool Database::check_if_in_db(fs::directory_entry entry)
{
    Statement statement(db);

    std::string query = std::format(
        "SELECT * FROM track WHERE filename = \"{0}\" AND modified = {1};",
        entry.path().filename().string(), 
        std::chrono::duration_cast<std::chrono::seconds>(fs::last_write_time(entry.path()).time_since_epoch()).count()
    );

    statement.prepare(query);

    if (statement.step() == SQLITE_ROW) return true;
    else return false;
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

    add_if_not_in_db();
    remove_if_not_in_music_dir();

    return 0;
}

void Database::add_if_not_in_db()
{
    for (const fs::directory_entry entry: fs::recursive_directory_iterator(musicPath))
    {
        if (check_if_in_db(entry))
        {
            logger.debug("Found matching entry for file {0} with modification time {1}, skipping!\n",
                entry.path().filename().string(), 
                fs::last_write_time(entry.path()).time_since_epoch().count()
            );
            continue;
        }
        std::shared_ptr<Track> track = std::make_shared<Track>(entry.path());
        if (track->type == TrackType::UNKNOWN)
        {
            continue;
        }

        if (track->read_from_file())
        {
            add_song(track);
        }
    }

    return;
}

void Database::remove_if_not_in_music_dir()
{
    Statement statement = tables->track->select_iter();
    
    while(statement.step() == SQLITE_ROW)
    {
        int id = statement.column_int(0);
        std::string filename = statement.column_text(2);
        long long modified = statement.column_int64(3);
        fs::path fullPath = musicPath / filename;

        if (fs::exists(fullPath))
        {
            if (std::chrono::duration_cast<std::chrono::seconds>(fs::last_write_time(fullPath).time_since_epoch()).count() == modified)
            {
                continue;
            }
        }

        remove_song(id);
    }
}

bool Database::add_song(std::shared_ptr<Track>& track)
{
    int trackNo = tables->track->insert(track);

    if (trackNo < 0)
    {
        return false;
    }

    if (track->sequence)
    {
        int seqNo = tables->relation.track_to_seq->select(trackNo);
        if (seqNo > 0)
        {
            if (tables->seq->update(seqNo, track->sequence) < 0) { return false; };
        }
        else
        {
            seqNo = tables->seq->insert(track->sequence);
            if (seqNo > 0)
            {
                tables->relation.track_to_seq->insert(trackNo, seqNo);
            }
            else return false;
        }
    }
    if (track->bank)
    {
        int bankNo = tables->relation.track_to_bank->select(trackNo);
        if (bankNo > 0)
        {
            if (tables->bank->update(bankNo, track->bank)) { return false; };
        }
        else
        {
            bankNo = tables->bank->insert(track->bank);
            if (bankNo > 0)
            {
                tables->relation.track_to_bank->insert(trackNo, bankNo);
            }
            else return false;
        }
    }
    
    int soundNo = 0;
    Statement statement = tables->relation.track_to_sound->select_iter(trackNo);
    for (int i = 0; (soundNo = statement.exec_and_return_id()) > 0; i++)
    {
        track->sounds[i]->databaseIndex = soundNo;
    }
    for (int i = 0; i < track->sounds.size(); i++) 
    {
        if (tables->sound->check_exists(track->sounds[i]->databaseIndex))
        {
            if (tables->sound->update(track->sounds[i]->databaseIndex, track->sounds[i]) < 0) { return false; };
        }
        else
        {
            int soundNo = tables->sound->insert(track->sounds[i]);
            if (soundNo > 0)
            {
                tables->relation.track_to_sound->insert(trackNo, soundNo);
            }
            else return false;
        }
    }
    
    return true;
}

bool Database::remove_song(int id)
{
    int returnedId = 0;

    if ((returnedId = tables->track->remove(id)) < 0)
    {
        return false;
    }
    if ((returnedId = tables->relation.track_to_seq->remove(returnedId)) > 0)
    {
        if ((returnedId = tables->seq->remove(returnedId) < 0))
        {
            logger.warning << "Found TrackToSeq entry but not seq entry for track id " << id << "!" << std::endl;
            return false;
        }
    }
    if ((returnedId = tables->relation.track_to_bank->remove(id)) > 0)
    {
        if ((returnedId = tables->bank->remove(returnedId) < 0))
        {
            logger.warning << "Found TrackToBank entry but not bank entry for track id " << id << "!" << std::endl;
            return false;
        }
    }
    
    Statement statement = tables->relation.track_to_sound->remove_iter(id);
    while ((returnedId = statement.exec_and_return_id()) > 0)
    {
        if ((returnedId = tables->sound->remove(returnedId) < 0))
        {
            logger.warning << "Found TrackToSound entry but not sound entry for track id " << id << "!" << std::endl;
            return false;
        }
    }

    return true;
}


int Database::load_all_songs()
{
    tables->track->load_entries();
    tables->seq->load_entries();
    /* Not ready yet
    tables->bank->load_entries();
    tables->sound->load_entries();

    tables->relation.track_to_seq->link_entries();
    tables->relation.track_to_bank->link_entries();
    tables->relation.track_to_sound->link_entries();

    */


   return 0;
}
