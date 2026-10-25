#include "table.hpp"
#include "database.hpp"

template<> ImageTable::Table(std::shared_ptr<Database> db): db(db), name("image")
{
    std::string query = 
        "CREATE TABLE IF NOT EXISTS image (                  "
        "id INTEGER PRIMARY KEY AUTOINCREMENT,               "
        "filepath TEXT UNIQUE,                               "
        "modified BIGINT,                                    "
        "data BLOB                                           "
    ");";

    db->exec(query);
}

template<> int ImageTable::insert(std::shared_ptr<Image> entry)
{
    Statement statement(get_sqlite());

    std::string query = 
        "INSERT INTO " + name + " ( \
             filepath,              \
             modified,              \
             data                   \
        )                           \
        VALUES (?, ?, ?)            \
        ON CONFLICT (filepath) DO   \
        UPDATE SET                  \
        data=?                      \
        RETURNING id                \
        ";
    
    if (statement.prepare(query))
    {
        return -2;
    }

    if (!entry->path.empty())
        statement.bind_text(entry->path.parent_path().string());
    else
        statement.bind_text("");
    statement.bind_blob(entry->rgba32, sizeof(u64) * 64 * 32);
    statement.bind_blob(entry->rgba32, sizeof(u64) * 64 * 32);

    int dbIdx = statement.exec_and_return_id();
    entry->databaseIndex = dbIdx;

    return dbIdx;
}

template<> void ImageTable::create_from_statement(Statement& statement, std::shared_ptr<Image> obj) 
{
    obj->databaseIndex = statement.column_int(0);
    obj->path = statement.column_text(1);
    obj->timestamp = statement.column_int64(2);
    obj->data = statement.column_blob(3);
}

template <> int ImageTable::update(int id, std::shared_ptr<Image> entry)
{
    Statement statement(get_sqlite());

    std::string query = 
        "UPDATE " +  name + "                           \
         SET  modified = ?, data = ?                    \
         WHERE (id = ?);                                \
        ";

    if (statement.prepare(query))
    {
        return -2;
    }

    statement.bind_int64(entry->timestamp);
    statement.bind_blob_vec(entry->data);
    statement.bind_int(id);

    int dbIdx = statement.exec_and_return_id();

    return dbIdx;
}