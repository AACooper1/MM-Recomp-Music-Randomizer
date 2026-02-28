#include "table.h"
#include "database.h"

template<> SequenceTable::Table(std::shared_ptr<Database> db) : db(db), name("seq")
{
    std::string query =
        "CREATE TABLE IF NOT EXISTS seq (             "
            "id INTEGER PRIMARY KEY AUTOINCREMENT,    "
            "size INTEGER,                            "
            "data BLOB                                "
        ");";

    db->exec(query);
}

template <> int SequenceTable::insert(std::shared_ptr<Sequence> entry)
{
    Statement statement(get_sqlite());

    std::string query = 
        "INSERT INTO seq (              \
            size,                       \
            data                        \
        )                               \
        VALUES (?, ?)                   \
        RETURNING id                    \
        ";

    if (statement.prepare(query))
    {
        return -2;
    }

    statement.bind_int(entry->size);
    statement.bind_blob_vec(*entry->data);

    int dbIdx = statement.exec_and_return_id();
    entry->databaseIndex = dbIdx;

    return dbIdx;
}

template <> int SequenceTable::update(int id, std::shared_ptr<Sequence> entry)
{
    Statement statement(get_sqlite());

    std::string query = 
        "UPDATE seq                     \
         SET (size = ?, data = ?)       \
         WHERE (id = ?);                \
        ";

    if (statement.prepare(query))
    {
        return -2;
    }

    statement.bind_int(entry->size);
    statement.bind_blob_vec(*entry->data);
    statement.bind_int(entry->databaseIndex);

    int dbIdx = statement.exec_and_return_id();

    return dbIdx;
}

template<> void SequenceTable::create_from_statement(Statement& statement, std::shared_ptr<Sequence> obj) 
{
    obj->databaseIndex = statement.column_int(0);
    obj->size = statement.column_int(1);
    *obj->data = statement.column_blob(2);
}