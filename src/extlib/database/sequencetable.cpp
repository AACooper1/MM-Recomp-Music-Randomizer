#include "table.h"
#include "database.h"

template<> SequenceTable::Table(std::shared_ptr<Database> db) : db(db)
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
    if (dbIdx < 0)
    {
        return dbIdx;
    }
    entry->databaseIndex = dbIdx;
    entries.emplace(entry->databaseIndex, entry);

    return 0;
}