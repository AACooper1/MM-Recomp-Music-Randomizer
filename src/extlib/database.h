#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"

#include <cstdarg>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <format>
#include <string>
#include <stdexcept>

#include "util.h"
#include "logging.h"
#include "track.h"

namespace fs = std::filesystem;
extern Log logger;

class AudioFile;
class Track;
class Table;

class Database
{
    public:
        static Database* get_db();
        static Database* get_db(fs::path path);
        sqlite3* sqlite() { return db; };

        bool add_track(std::unique_ptr<Track>& track);

        int update_from_music_dir();

        int exec(std::string query);

        void set_last_rc(int rc);
        int get_last_rc();
        
        char* lastErrMsg;
        
        dbTables tables;

    protected:
        Database(fs::path path);
        ~Database();

        sqlite3* db;

        sqlite3_stmt* statement;
        static Database* database_;
        int lastRC = 0;
        std::stringstream errMsg;

        fs::path dbPath;
        fs::path musicPath;

        void init_tables();
        void report_error();
};

struct dbTables
{
    std::unique_ptr<TrackTable> track;
    std::unique_ptr<SequenceTable> seq;
    std::unique_ptr<BankTable> bank;
    std::unique_ptr<SoundTable> sound;

    RelationTables relation;
};

struct RelationTables
{
    std::unique_ptr<TrackToSequenceTable> track_to_seq;
    std::unique_ptr<TrackToBankTable> track_to_bank;
    std::unique_ptr<TrackToSoundTable> track_to_sound;
};


template <typename T>
class Table
{
    public:
        Table(Database* db);

        bool insert(std::unique_ptr<T> entry) override;
        bool update(std::unique_ptr<T> entry) override;
        bool upsert(std::unique_ptr<T> entry) override { entry.upsert(this); };

        bool remove(std::string query) override;
        bool remove(int id) override;

    protected:
        virtual int exec(std::string query);
        sqlite3* get_sqlite() { return db->sqlite(); };

        std::shared_ptr<Database> db;
        int rc = 0;

        std::string name;
        int n_col;
        int n_rows;

        std::vector<std::string> cols;
};


template <typename T> class Table_Impl : public Table
{
    public:
        TableImpl(Database* db);

        bool insert(std::unique_ptr<T> entry) override;
        bool update(std::unique_ptr<T> entry) override;
        bool upsert(std::unique_ptr<T> entry) override { entry.upsert(this); };

        bool remove(std::string query) override;
        bool remove(int id) override;
    protected:
        std::unordered_map<std::string, Track> entries;
};

class TrackTable : public Table<Track>
{
    public:
        TrackTable(std::shared_ptr<Database> db) : Table<Track>(db)
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
};


template<typename T> class AudioFileTable : public Table<AudioFile>
{
    public:
        AudioFileTable(std::shared_ptr<Database> db);
        
        T* select(std::string query) override;
        T* select(std::string query, std::string* cols) override;
        T* select(int id) override

        bool insert(std::unique_ptr<T> entry);
        bool update(std::unique_ptr<T> entry);
        bool upsert(std::unique_ptr<T> entry);

        bool remove(std::string query) override;
        bool remove(int id) override;
    
    private:
        std::vector<T> entries;
};

class SequenceTable : public AudioFileTable<Sequence>
{
    public:
        SequenceTable(std::shared_ptr<Database> db) : AudioFileTable(db)
        {
            std::string query =
                "CREATE TABLE IF NOT EXISTS seq (             "
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,    "
                    "size INTEGER,                            "
                    "data BLOB                                "
                ");";

            db->exec(query);
        }
};

class BankTable : public AudioFileTable<Bank>
{
    public:
        BankTable(std::shared_ptr<Database> db): AudioFileTable(db)
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
};

class SoundTable : public AudioFileTable<Sound>
{
    public:
        SoundTable(std::shared_ptr<Database> db): AudioFileTable(db)
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
};

class Relation;

class RelationTable : public Table<Relation>
{
    public:
        virtual bool insert(int id_1, int id_2) = 0;
        virtual int select(int id) = 0;

        RelationTable(Database* db, std::string name, std::string primaryKey, std::string cols...);

    private:
        std::string col1;
        std::string col2;
};

class TrackToSequenceTable : public RelationTable
{
    public:
        TrackToSequenceTable(Database* db, std::string name, std::string primaryKey, std::string cols...) :
        RelationTable(db, name, primaryKey, std::forward<std::string>(cols)) {}

        bool insert(int id_1, int id_2) override;
        int select(int id) override;
        bool remove(int id) override;
        bool remove(std::string id) override;
};

class TrackToBankTable : public RelationTable
{
    public:
        
        TrackToBankTable(Database* db, std::string name, std::string primaryKey, std::string cols...) :
        RelationTable(db, name, primaryKey, std::forward<std::string>(cols)) {}
    
        bool insert(int id_1, int id_2) override;
        int select(int id) override;
        bool remove(int id) override;
        bool remove(std::string id) override;
};

class TrackToSoundTable : public RelationTable
{
    public:
        TrackToSoundTable(Database* db, std::string name, std::string primaryKey, std::string cols...):
        RelationTable(db, name, primaryKey, std::forward<std::string>(cols)) {}

        bool insert(int id_1, int id_2) override;
        int select(int id) override;
        bool remove(int id) override;
        bool remove(std::string id) override;
};

#endif