#ifndef TABLES_H
#define TABLES_H

#include "sqlite3.h"
#include <unordered_map>

#include "track.h"

class Database;

template <typename T>
class Table
{
    public:
        Table<T>(std::shared_ptr<Database> db) {this->db = db;}

        virtual std::shared_ptr<T> select(std::string query);
        virtual std::shared_ptr<T> select(std::string query, std::string* cols);
        virtual std::shared_ptr<T> select(int id);

        virtual int insert(std::shared_ptr<T> entry);
        virtual bool update(std::shared_ptr<T> entry);

        virtual bool remove(std::string query);
        virtual bool remove(int id);

    protected:
        virtual int exec(std::string query);
        std::shared_ptr<sqlite3> get_sqlite();

        std::shared_ptr<Database> db;
        int rc = 0;

        std::string name;
        int n_col;
        int n_rows;

        std::vector<std::string> cols;
        std::unordered_map<int, std::shared_ptr<T>> entries;
};

using TrackTable = Table<Track>;
template<> TrackTable::Table(std::shared_ptr<Database> db);
template<> int TrackTable::insert(std::shared_ptr<Track> entry);

using SequenceTable = Table<Sequence>;
template<> SequenceTable::Table(std::shared_ptr<Database> db);
template<> int SequenceTable::insert(std::shared_ptr<Sequence> entry);

using BankTable = Table<Bank>;
template<> BankTable::Table(std::shared_ptr<Database> db);
template<> int BankTable::insert(std::shared_ptr<Bank> entry);

using SoundTable = Table<Sound>;
template<> SoundTable::Table(std::shared_ptr<Database> db);
template<> int SoundTable::insert(std::shared_ptr<Sound> entry);

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

#endif