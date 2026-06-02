#include "table.h"
#include "database.h"

template<> BankTable::Table(std::shared_ptr<Database> db, bool is_oot): db(db), name("ootbank")
{
    std::string query =
        "CREATE TABLE IF NOT EXISTS ootbank (                   "
            "id INTEGER PRIMARY KEY AUTOINCREMENT,           "
            "headerSize INTEGER,                             "
            "header BLOB,                                    "
            "dataSize INTEGER,                               "
            "data BLOB                                       "
        ");";

    db->exec(query);
}

template<> SoundTable::Table(std::shared_ptr<Database> db, bool is_oot): db(db), name("ootsound")
{
    std::string query =
            "CREATE TABLE IF NOT EXISTS ootsound (              "
            "id INTEGER PRIMARY KEY AUTOINCREMENT,           "
            "size INTEGER,                                   "
            "foreignKey INTEGER,                             "
            "data BLOB                                       "
        ");";

    db->exec(query);
}