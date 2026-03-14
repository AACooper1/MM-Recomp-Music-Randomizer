#include "table.h"
#include "database.h"

template<> SoundTable::Table(std::shared_ptr<Database> db): db(db), name("sound")
{
    std::string query =
            "CREATE TABLE IF NOT EXISTS sound (              "
            "id INTEGER PRIMARY KEY AUTOINCREMENT,           "
            "size INTEGER,                                   "
            "foreignKey INTEGER,                             "
            "data BLOB                                       "
        ");";

    db->exec(query);
}

template <> int SoundTable::insert(std::shared_ptr<Sound> entry)
{
    Statement statement(get_sqlite());

    std::string query = 
        "INSERT INTO sound (            \
            size,                       \
            foreignKey,                 \
            data                        \
        )                               \
        VALUES (?, ?, ?)                \
        RETURNING id                    \
        ";

    if (statement.prepare(query))
    {
        return -2;
    }

    statement.bind_int(entry->size);
    statement.bind_int(entry->sampleAddr);
    statement.bind_blob_vec(*entry->data);

    int dbIdx = statement.exec_and_return_id();
    entry->databaseIndex = dbIdx;

    return dbIdx;
}

template<> void SoundTable::create_from_statement(Statement& statement, std::shared_ptr<Sound> obj) 
{
    obj->databaseIndex = statement.column_int(0);
    obj->size = statement.column_int(1);
    obj->sampleAddr = statement.column_int(2);
    *obj->data = statement.column_blob(3);
}

template <> int SoundTable::update(int id, std::shared_ptr<Sound> entry)
{
    Statement statement(get_sqlite());

    std::string query = 
        "UPDATE sound                                   \
         SET  size = ?, foreignKey = ?, data = ?        \
         WHERE (id = ?);                                \
        ";

    if (statement.prepare(query))
    {
        return -2;
    }

    statement.bind_int(entry->size);
    statement.bind_blob_vec(*entry->data);
    statement.bind_int64(entry->sampleAddr);
    statement.bind_int(id);

    int dbIdx = statement.exec_and_return_id();

    return dbIdx;
}