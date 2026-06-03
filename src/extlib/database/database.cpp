#include "database.h"

Database::Database(fs::path path)
{
    this->dbPath = path;
    this->musicPath = dbPath / "music";

    if (!fs::exists(this->musicPath))
    {
        fs::create_directories(this->musicPath);
        logger.info << "Created new directory " << musicPath << "." << std::endl;
    }

    this->dbPath /= "music.db";
    
    std::string dbPath_forSql = dbPath.string();

    lastErrMsg = new char;

    sqlite3* dbPtrRaw = nullptr;

    int rc = sqlite3_open(dbPath_forSql.c_str(), &dbPtrRaw);

    set_last_rc(rc);

    this->db.reset(dbPtrRaw, sqlite3_close);
    this->tables = std::make_unique<dbTables>();
}

Database::Database(fs::path path, bool is_seed_db)
{
    this->dbPath = path;
    
    std::string dbPath_forSql = dbPath.string();

    lastErrMsg = new char;

    sqlite3* dbPtrRaw = nullptr;

    int rc = sqlite3_open(dbPath_forSql.c_str(), &dbPtrRaw);

    set_last_rc(rc);

    this->db.reset(dbPtrRaw, sqlite3_close);
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
        fs::relative(entry.path(), musicPath).string(),
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
    tables->relation.oot_bank_to_bank = std::make_unique<OoTBankToBankTable>(shared_from_this(), "oot_bank_to_bank");
    tables->relation.oot_bank_to_sound = std::make_unique<OoTBankToSoundTable>(shared_from_this(), "oot_bank_to_sound");
}

int Database::update_from_music_dir()
{
    if (!fs::exists(musicPath))
    {
        return 2;
    }

    add_if_not_in_db();
    remove_if_not_in_music_dir();

    logger.debug << "Finished update_from_music_dir" << std::endl;

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
            logger.debug.disable_header();
            continue;
        }
        logger.debug.enable_header();
        std::shared_ptr<Track> track = std::make_shared<Track>(entry.path());
        if (track->type == TrackType::UNKNOWN)
        {
            logger.error << "Got unknown track type " << (int)track->type << "for song " << track->name << "! Song will be skipped." << std::endl;
            continue;
        }
        if (track->type == TrackType::OOTRS && !allow_add_ootrs)
        {
            logger.warning << "Skipping OoTRS song " << track->name << ", as OoT audiobin is not present." << std::endl;
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
    track->path = fs::relative(track->path, musicPath);
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
            track->seqId = tables->seq->insert(track->sequence);
            if (track->seqId > 0)
            {
                tables->relation.track_to_seq->insert(trackNo, track->seqId);
            }
            else return false;
        }
    }
    if (track->bank)
    {
        track->bankId = tables->relation.track_to_bank->select(trackNo);
        if (track->bankId > 0)
        {
            if (tables->bank->update(track->bankId, track->bank)) { return false; };
        }
        else
        {
            track->bankId = tables->bank->insert(track->bank);
            if (track->bankId > 0)
            {
                tables->relation.track_to_bank->insert(trackNo, track->bankId);
            }
            else return false;
        }
    }

    if (track->type == TrackType::OOTRS && track->bank)
    {
        if (!ootAudioHandler)
        {
            logger.dev << "New OoTRS detected. Reading OoT audiobin..." << std::endl;
            ootAudioHandler = new OoTAudioHandler(get_db_dir() / "OOT.audiobin");
            ootAudioHandler->prepare_oot_audio();
            if (add_oot_banks(ootAudioHandler))
            {
                logger.info << "Failed to add OoT banks to database, song will be skipped!" << std::endl;
                return false;
            }
        }
        track->fix_oot_custom_bank(ootAudioHandler);
        tables->bank->update(track->bankId, track->bank);
    }

    int soundNo = 0;
    Statement statement = tables->relation.track_to_sound->select_iter(trackNo);
    for (int i = 0; (soundNo = statement.exec_and_return_id()) > 0; i++)
    {
        track->sounds[i]->databaseIndex = soundNo;
        track->soundIds.push_back(soundNo);
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

/* Loads all tracks, leaves their seq/bank/sounds empty. */
int Database::load_all_tracks()
{
    tables->track->load_entries();

    // I don't think this should ever have to run or even be reachable
    // But I used "Jump to Cursor" for the first time and can confirm it works
    // Eat your heart out, Edsger "Goto Considered Harmful" Dijkstra
    if (!allow_use_ootrs)
    {
        std::erase_if(tables->track->entries, [](const auto& pair) {
            return pair.second->type == TrackType::OOTRS;
        });
    }

    for (const auto & [id, track] : tables->track->entries)
    {
        if (track->type == TrackType::OOTRS && track->bankNo < 0x26)
        {            
            Statement statement = tables->relation.oot_bank_to_sound->select_iter(track->bankNo);
            while (statement.step() == SQLITE_ROW)
            {
                track->soundIds.push_back(statement.column_int(1));
            }
            
            track->bankId = tables->relation.oot_bank_to_bank->select(track->bankNo);
        }
        else
        {
            track->bankId = tables->relation.track_to_bank->select(id);
        }

        track->seqId = tables->relation.track_to_seq->select(id);
        
        Statement statement = tables->relation.track_to_sound->select_iter(id);
        while (statement.step() == SQLITE_ROW)
        {
            track->soundIds.push_back(statement.column_int(1));
        }
    }

    return tables->track->entries.size();
}

/* This generally shouldn't be used - only load necessary songs where possible */
int Database::load_all_songs()
{
    tables->track->load_entries();
    tables->seq->load_entries();
    tables->bank->load_entries();
    tables->sound->load_entries();

   return tables->track->entries.size();
}

int Database::prepare_track(int id)
{
    std::shared_ptr<Track> track = tables->track->entries[id];
    if (track)
    {
        logger.debug << "Preparing track " << track->name << "..." << std::endl;
        if (track->seqId)
        {
            tables->seq->load_entry(track->seqId);
            track->sequence = tables->seq->entries[track->seqId];
        }
        if (track->bankId > 0)
        {
            tables->bank->load_entry(track->bankId);
            track->bank = tables->bank->entries[track->bankId];
        }
        for (int i = 0; i < track->soundIds.size(); i++)
        {
            tables->sound->load_entry(track->soundIds[i]);
            track->sounds.push_back(tables->sound->entries[track->soundIds[i]]);
        }
        track->is_prepared = true;
        logger.debug << "Track " << track->name << " prepared!" << std::endl;
        return true;
    }
    else
    {
        logger.error << "Could not prepare track, returned NULL." << std::endl;
        return false;
    }
}

int Database::add_oot_banks(OoTAudioHandler* audioHandler)
{
    if (audioHandler->ootBanks.size() != 0x26)
    {
        logger.error << "OoTBanks should have 38 entries, but had " << audioHandler->ootBanks.size() << "!" << std::endl;
        return 1;
    }
    for (int bankNo = 0; bankNo < audioHandler->ootBanks.size(); bankNo++)
    {
        AudioBank& parsed_bank = audioHandler->ootBanks[bankNo];

        std::shared_ptr<Bank> bank = std::make_shared<Bank>(parsed_bank);

        int bankId = this->tables->bank->insert(bank);
        this->tables->relation.oot_bank_to_bank->insert(bankNo, bankId);

        for (int soundNo = 0; soundNo < parsed_bank.zsounds_to_add.size(); soundNo++)
        {
            Sample* parsed_sample = parsed_bank.zsounds_to_add[soundNo];
            std::shared_ptr<Sound> sound = std::make_shared<Sound>(parsed_sample);

            int soundId = this->tables->sound->insert(sound);
            this->tables->relation.oot_bank_to_sound->insert(bankNo, soundId);
        }
    }

    return 0;
}