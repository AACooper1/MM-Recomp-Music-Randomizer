#include "table.h"
#include "database.h"

RelationTable::RelationTable(std::shared_ptr<Database> db, std::string name)
{
    this->db = db;
    this->name = name;
}

int RelationTable::init()
{
    std::string query = std::format(
        "CREATE TABLE IF NOT EXISTS {0}( "
            "{1} INTEGER PRIMARY KEY,    "
            "{2} INTEGER                 "
            ")",
        name, col1, col2
    );

    return db->exec(query);
}

// Remember: id_1 should be soundId for track_to_sound table
int RelationTable::insert(int id_1, int id_2)
{
    std::string query = std::format(
        "INSERT INTO {0} ({1}, {2})"
        "VALUES ({3}, {4})",
        name, col1, col2, id_1, id_2
    );

    return db->exec(query);
}

int RelationTable::remove(int id) { }