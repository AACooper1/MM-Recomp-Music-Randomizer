#include "table.h"
#include "database.h"

template<> BankTable::Table(std::shared_ptr<Database> db): db(db), name("bank")
{
    std::string query =
        "CREATE TABLE IF NOT EXISTS bank (                   "
            "id INTEGER PRIMARY KEY AUTOINCREMENT,           "
            "headerSize INTEGER,                             "
            "header BLOB,                                    "
            "dataSize INTEGER,                               "
            "data BLOB                                       "
        ");";

    db->exec(query);
}

template <> int BankTable::insert(std::shared_ptr<Bank> entry)
{
    Statement statement(get_sqlite());

    std::string query = 
        "INSERT INTO bank (             \
            headerSize,                 \
            header,                     \
            dataSize,                   \
            data                        \
        )                               \
        VALUES (?, ?, ?, ?)             \
        RETURNING id                    \
        ";

    if (statement.prepare(query))
    {
        return -2;
    }

    statement.bind_int(entry->header->size());
    statement.bind_blob_vec(*entry->header);
    statement.bind_int(entry->size);
    statement.bind_blob_vec(*entry->data);

    
    int dbIdx = statement.exec_and_return_id();

    return dbIdx;
}

template <> int BankTable::update(int id, std::shared_ptr<Bank> entry)
{
    Statement statement(get_sqlite());

    std::string query = 
        "UPDATE bank                        \
         SET (                              \
            headerSize = ?, headerData = ?, \
            dataSize = ?, data = ?)         \
         WHERE (id = ?);                    \
        ";

    if (statement.prepare(query))
    {
        return -2;
    }

    statement.bind_int(entry->header->size());
    statement.bind_blob_vec(*entry->header);
    statement.bind_int(entry->size);
    statement.bind_blob_vec(*entry->data);
    statement.bind_int(entry->databaseIndex);

    int dbIdx = statement.exec_and_return_id();

    return dbIdx;
}
