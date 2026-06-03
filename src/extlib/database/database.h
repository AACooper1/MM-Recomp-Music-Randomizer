#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"

#include <cstdarg>
#include <filesystem>

#include <vector>
#include <format>
#include <string>
#include <stdexcept>

#include "util.h"
#include "logging.hpp"
#include "table.h"


namespace fs = std::filesystem;
extern Log logger;

template <typename T>
class Table;
struct RelationTables;
struct dbTables;

class Track;

struct Statement
{
    Statement(std::shared_ptr<sqlite3> db) { this->db = db; };
    ~Statement() { sqlite3_finalize(statement);}

    int prepare(std::string query) 
        { return sqlite3_prepare_v2(db.get(), query.c_str(), -1, &statement, nullptr); }

    int step()
        { return sqlite3_step(statement); }

    int bind_blob_vec(std::vector<char> data)
        { i++; return sqlite3_bind_blob(statement, i, data.data(), data.size(), SQLITE_TRANSIENT); }
    int bind_blob(void* data, int size)
        { i++; return sqlite3_bind_blob(statement, i, data, size, SQLITE_STATIC);}
    int bind_int(int value)
        { i++; return sqlite3_bind_int(statement, i, value); }
    int bind_int64(long long value)
        { i++; return sqlite3_bind_int64(statement, i, value); }
    int bind_text(std::string text)
        { i++; return sqlite3_bind_text(statement, i, text.c_str(), text.length(), SQLITE_TRANSIENT); }

    std::string column_text(int col)
        { return reinterpret_cast<const char*>(sqlite3_column_text(statement, col)); }
    int column_int(int col)
        { return sqlite3_column_int(statement, col); }
    long long column_int64(int col)
        { return sqlite3_column_int64(statement, col); }
    std::vector<char> column_blob(int col)
        { 
            std::vector<char> retvec;

            const char* buffer = reinterpret_cast<const char*>(sqlite3_column_blob(statement, col));
            size_t size = sqlite3_column_bytes(statement, col);

            for (int i = 0; i < size; i++)
            {
                retvec.push_back(buffer[i]);
            }

            return retvec;
        }
    
    
    int exec_and_return_id()
    {
        if ((sqlite3_step(statement)) == SQLITE_ROW)
        {
            return sqlite3_column_int(statement, 0);
        }
        else
        {
            return -1;
        }
    }

    sqlite3_stmt* statement;
    std::shared_ptr<sqlite3> db;
    int i = 0;
};

class Database : public std::enable_shared_from_this<Database>
{
    public:
        Database(fs::path path);
        Database(fs::path path, bool is_seed_db);
        ~Database();

        std::shared_ptr<sqlite3> sqlite() { return db; };

        bool add_song(std::shared_ptr<Track>& track);
        bool remove_song(int id);

        void init();
        int update_from_music_dir();

        int load_all_tracks();
        int load_all_songs();
        int add_oot_banks(OoTAudioHandler* audioHandler);

        int prepare_track(int id);

        int exec(std::string query);

        fs::path get_db_dir() { return dbPath.parent_path(); }

        void set_last_rc(int rc);
        int get_last_rc() { return lastRC; };

        OoTAudioHandler* ootAudioHandler;
        bool allow_use_ootrs = false;
        bool allow_add_ootrs = false;
        
        char* lastErrMsg;
        
        std::unique_ptr<dbTables> tables;
        std::unique_ptr<RelationTable> seedTable;

    protected:
        std::shared_ptr<sqlite3> db;

        int lastRC = 0;
        std::stringstream errMsg;

        fs::path dbPath;
        fs::path musicPath;

        void add_if_not_in_db();
        void remove_if_not_in_music_dir();

        bool check_if_in_db(fs::directory_entry entry);

        void init_tables();
        void report_error();
};


struct RelationTables
{
    std::unique_ptr<TrackToSequenceTable> track_to_seq;
    std::unique_ptr<TrackToBankTable> track_to_bank;
    std::unique_ptr<TrackToSoundTable> track_to_sound;
    std::unique_ptr<OoTBankToBankTable> oot_bank_to_bank;
    std::unique_ptr<OoTBankToSoundTable> oot_bank_to_sound;
};

struct dbTables
{
    std::unique_ptr<TrackTable> track;
    std::unique_ptr<SequenceTable> seq;
    std::unique_ptr<BankTable> bank;
    std::unique_ptr<SoundTable> sound;

    RelationTables relation;

    std::unique_ptr<BankTable> ootbank;
    std::unique_ptr<SoundTable> ootsound;
};

#endif