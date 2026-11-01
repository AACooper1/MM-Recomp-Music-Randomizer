#include "table.hpp"
#include "database.hpp"

template<> ConfigTable::Table(std::shared_ptr<Database> db) : db(db), name("config")
{
    std::string query =         
        "CREATE TABLE IF NOT EXISTS config (         \
            id INTEGER PRIMARY KEY,                  \
            categories BLOB,                         \
            volume REAL,                             \
            weight REAL,                             \
            is_songforce INT,                        \
            is_songtest INT                          \
        );";

    db->exec(query);
}

template<> int ConfigTable::insert(std::shared_ptr<Config> entry) 
{
    Statement statement(get_sqlite());

    std::string query = 
        "INSERT INTO config (           \
            id,                         \
            categories,                 \
            volume,                     \
            weight,                     \
            is_songforce,               \
            is_songtest                 \
        )                               \
        VALUES (?, ?, ?, ?, ?, ?)       \
        ON CONFLICT (id) DO             \
        UPDATE SET                      \
            categories=?,               \
            volume=?,                   \
            weight=?,                   \
            is_songforce=?,             \
            is_songtest=?               \
        RETURNING id;";

    if (statement.prepare(query))
    {
        return -2;
    }

    statement.bind_int(entry->trackId);
    statement.bind_blob_vec(*entry->categories);
    statement.bind_double(entry->volume);
    statement.bind_double(entry->weight);
    statement.bind_int(entry->is_songforce);
    statement.bind_int(entry->is_songtest);

    statement.bind_blob_vec(*entry->categories);
    statement.bind_double(entry->volume);
    statement.bind_double(entry->weight);
    statement.bind_int(entry->is_songforce);
    statement.bind_int(entry->is_songtest);

    return statement.exec_and_return_id();
}

template<> void ConfigTable::create_from_statement(Statement& statement, std::shared_ptr<Config> obj)
{
        obj->trackId = statement.column_int(0);
        *obj->categories = statement.column_blob(1);
        obj->volume = statement.column_double(2);
        obj->weight = statement.column_double(3);
        obj->is_songforce = statement.column_int(4);
        obj->is_songtest = statement.column_int(5);
}

template<> int ConfigTable::update(int id, std::shared_ptr<Config> entry)
{
    Statement statement(get_sqlite());

    std::string query = 
        "UPDATE config          \
         SET                    \
            categories = ?,     \
            volume = ?          \
            weight = ?,         \
            is_songforce = ?,   \
            is_songtest = ?     \
         WHERE (id = ?)         \
         RETURNING id;          \
        ";

    if (statement.prepare(query))
    {
        return -2;
    }

    statement.bind_blob_vec(*entry->categories);
    statement.bind_double(entry->volume);
    statement.bind_double(entry->weight);
    statement.bind_int(entry->is_songforce);
    statement.bind_int(entry->is_songtest);

    statement.bind_int(entry->trackId);
}