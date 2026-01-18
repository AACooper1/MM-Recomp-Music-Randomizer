#include "table.h"
#include "database.h"

template<> TrackTable::Table(std::shared_ptr<Database> db) : db(db), name("track")
{
    std::string query =         
        "CREATE TABLE IF NOT EXISTS track (          \
            id INTEGER PRIMARY KEY AUTOINCREMENT,    \
            filename TEXT UNIQUE,                    \
            modified BIGINT,                         \
            songName TEXT,                           \
            categories BLOB,                         \
            bankNo INTEGER,                          \
            formMask BLOB                            \
        );";

    db->exec(query);
}

template<> int TrackTable::insert(std::shared_ptr<Track> entry) 
{
    Statement statement(get_sqlite());

    std::string query = 
        "INSERT INTO track (            \
            filename,                   \
            modified,                   \
            songName,                   \
            categories,                 \
            bankNo,                     \
            formMask                    \
        )                               \
        VALUES (?, ?, ?, ?, ?, ?)       \
        ON CONFLICT (filename) DO       \
        UPDATE SET                      \
            modified=?,                 \
            songName=?,                 \
            categories=?,               \
            bankNo=?,                   \
            formMask=?                  \
        RETURNING id;";

    if (statement.prepare(query))
    {
        return -2;
    }

    statement.bind_text(entry->path.string());
    statement.bind_int64(entry->timestamp);
    statement.bind_text(entry->name);
    statement.bind_blob_vec(*entry->categories);
    statement.bind_int(entry->bankNo);
    statement.bind_blob(&entry->formmask.states, sizeof(FormMask));

    statement.bind_int64(entry->timestamp);
    statement.bind_text(entry->name);
    statement.bind_blob_vec(*entry->categories);
    statement.bind_int(entry->bankNo);
    statement.bind_blob(&entry->formmask.states, sizeof(FormMask));

    int dbIdx = statement.exec_and_return_id();
    if (dbIdx < 0)
    {
        return dbIdx;
    }
    entry->databaseIndex = dbIdx;
    entries.emplace(entry->databaseIndex, entry);

    return dbIdx;
}