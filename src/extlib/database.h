#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"

#include <cstdarg>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <format>
#include <string>

namespace fs = std::filesystem;

class AudioFile;
class Track;

class Database
{
    public:
        static Database* get_db();
        static Database* get_db(fs::path path);
        static sqlite3* sqlite() { return db; };

        int exec(std::string query);

        void set_last_rc(int rc);
        int get_last_rc();

        char* lastErrMsg = 0;
        
        std::unordered_map<std::string, Table*> tables;

    protected:
        Database(fs::path path);
        ~Database();

        static sqlite3* db;

        sqlite3_stmt* statement;
        static Database* database_;
        int lastRC = 0;

        fs::path path;

        bool init_tables();
};


class Table
{
    public:
        bool init(std::string query);

        virtual bool remove(std::string query);
        virtual bool remove(int id);
        virtual bool remove(std::string name);

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

        static TrackTable* table;
};





class AudioFileTable : public Table
{
    public:
        virtual bool insert(AudioFile file);

        virtual AudioFile* select(std::string query);
        virtual AudioFile select(std::string query, std::string* cols);
        virtual AudioFile select(int id);
};

class SequenceTable : public AudioFileTable
{
    public:
        SequenceTable(Database* db);
};

class BankTable : public AudioFileTable
{
    public:
        BankTable(Database* db);
};

class SoundTable : public AudioFileTable
{
    public:
        SoundTable(Database* db);
};




class RelationTable : public Table
{
    public:
        virtual bool insert(int id_1, int id_2);
        virtual int select(int id);
        virtual bool remove(int id);

        RelationTable(Database* db, std::string name, std::string primaryKey, std::string cols...);

    private:
        std::string col1;
        std::string col2;
};

class TrackToSequenceTable : public RelationTable
{
    public:
        TrackToSequenceTable(Database* db);
};

class TrackToBankTable : public RelationTable
{
    public:
        
        TrackToBankTable(Database* db);
    
};

class TrackToSoundTable : public RelationTable
{
    public:
        TrackToSoundTable(Database* db);
};

#endif