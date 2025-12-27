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

        bool add_track(Track* track);

        int update_from_music_dir();

        int exec(std::string query);

        void set_last_rc(int rc);
        int get_last_rc();
        
        char* lastErrMsg;
        std::unordered_map<std::string, Table*> tables;

    protected:
        Database(fs::path path);
        ~Database();

        bool add_mmrs(Track* track);
        bool add_ootrs(Track* file);
        bool add_streamed(Track* track);

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


class Table
{
    public:
        void init(std::string query);

        virtual bool remove(std::string query) = 0;
        virtual bool remove(int id) = 0;

    protected:
        virtual int exec(std::string query);
        sqlite3* get_sqlite() { return db->sqlite(); };

        Database* db;
        int rc = 0;

        std::string name;
        int n_col;
        int n_rows;

        std::vector<std::string> cols;
};




class TrackTable : public Table
{
    public:
        TrackTable(Database* db);

        bool add(Track* track);

        bool remove(std::string query) override;
        bool remove(int id) override;

        static TrackTable* table;
};





class AudioFileTable : public Table
{
    public:
        // virtual bool insert(AudioFile file);

        virtual AudioFile* select(std::string query) = 0;
        virtual AudioFile* select(std::string query, std::string* cols) = 0;
        virtual AudioFile* select(int id) = 0;
};

class SequenceTable : public AudioFileTable
{
    public:
        SequenceTable(Database* db);

        AudioFile* select(std::string query) override;
        AudioFile* select(std::string query, std::string* cols) override;
        AudioFile* select(int id) override;

        bool remove(std::string query) override;
        bool remove(int id) override;
};

class BankTable : public AudioFileTable
{
    public:
        BankTable(Database* db);

        AudioFile* select(std::string query) override;
        AudioFile* select(std::string query, std::string* cols) override;
        AudioFile* select(int id) override;

        bool remove(std::string query) override;
        bool remove(int id) override;
};

class SoundTable : public AudioFileTable
{
    public:
        SoundTable(Database* db);

        AudioFile* select(std::string query) override;
        AudioFile* select(std::string query, std::string* cols) override;
        AudioFile* select(int id) override;

        bool remove(std::string query) override;
        bool remove(int id) override;
};




class RelationTable : public Table
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