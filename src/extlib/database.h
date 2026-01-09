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
template <typename T>
class Table;
struct RelationTables;
struct dbTables;

class Database : public std::enable_shared_from_this<Database>
{
    public:
        Database(fs::path path);
        ~Database();

        std::shared_ptr<sqlite3> sqlite() { return db; };

        bool add_track(std::unique_ptr<Track>& track);

        int update_from_music_dir();
        void init();

        int exec(std::string query);
        template <typename ...T>
        void bind(sqlite3_stmt* statement, std::string query, T&&... args);

        void set_last_rc(int rc);
        int get_last_rc();
        
        char* lastErrMsg;
        
        std::unique_ptr<dbTables> tables;

    protected:
        std::shared_ptr<sqlite3> db;

        int lastRC = 0;
        std::stringstream errMsg;

        fs::path dbPath;
        fs::path musicPath;

        void init_tables();
        void report_error();
};

template <typename T>
class Table
{
    public:
        Table<T>(std::shared_ptr<Database> db) {this->db = db;}

        virtual std::shared_ptr<T> select(std::string query);
        virtual std::shared_ptr<T> select(std::string query, std::string* cols);
        virtual std::shared_ptr<T> select(int id);

        virtual int insert(std::unique_ptr<T> entry);
        virtual bool update(std::unique_ptr<T> entry);

        virtual bool remove(std::string query);
        virtual bool remove(int id);

    protected:
        virtual int exec(std::string query);
        std::shared_ptr<sqlite3> get_sqlite() { return db->sqlite(); };

        std::shared_ptr<Database> db;
        int rc = 0;

        std::string name;
        int n_col;
        int n_rows;

        std::vector<std::string> cols;
        std::vector<T> entries;
};

template<typename T> concept isAudioFile = std::is_base_of<AudioFile, T>().value;

using TrackTable = Table<Track>;
template<> TrackTable::Table(std::shared_ptr<Database> db);
template<> int TrackTable::insert(std::unique_ptr<Track> entry);

using SequenceTable = Table<Sequence>;
template<> SequenceTable::Table(std::shared_ptr<Database> db);
using BankTable = Table<Bank>;
template<> BankTable::Table(std::shared_ptr<Database> db);
using SoundTable = Table<Sound>;
template<> SoundTable::Table(std::shared_ptr<Database> db);


struct Relation
{
    int id_1;
    int id_2;
};

class RelationTable
{
    public:
        RelationTable(std::shared_ptr<Database> db, std::string name);
        virtual bool insert(int id_1, int id_2) = 0;
        virtual int select(int id) = 0;

        virtual bool remove(int id);

    protected:
        std::string col1;
        std::string col2;

        std::shared_ptr<Database> db;
};

class TrackToSequenceTable : public RelationTable
{
    public:
        
        TrackToSequenceTable(std::shared_ptr<Database> db, std::string name):
        RelationTable(db, name) {col1 = "trackId"; col2 = "sequenceId";}
    
        bool insert(int id_1, int id_2) override;
        int select(int id) override;
        bool remove(int id) override;
};

class TrackToBankTable : public RelationTable
{
    public:
        
        TrackToBankTable(std::shared_ptr<Database> db, std::string name):
        RelationTable(db, name) {col1 = "trackId"; col2 = "bankId";}
    
        bool insert(int id_1, int id_2) override;
        int select(int id) override;
        bool remove(int id) override;
};

class TrackToSoundTable : public RelationTable
{
    public:
        TrackToSoundTable(std::shared_ptr<Database> db, std::string name):
        RelationTable(db, name) {col1 = "trackId"; col2 = "soundId";}

        bool insert(int id_1, int id_2) override;
        int select(int id) override;
        bool remove(int id) override;
};

struct RelationTables
{
    std::unique_ptr<TrackToSequenceTable> track_to_seq;
    std::unique_ptr<TrackToBankTable> track_to_bank;
    std::unique_ptr<TrackToSoundTable> track_to_sound;
};

struct dbTables
{
    std::unique_ptr<TrackTable> track;
    std::unique_ptr<SequenceTable> seq;
    std::unique_ptr<BankTable> bank;
    std::unique_ptr<SoundTable> sound;

    RelationTables relation;
};


#endif