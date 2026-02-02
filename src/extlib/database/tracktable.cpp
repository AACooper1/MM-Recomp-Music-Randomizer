#include "table.h"
#include "database.h"

template<> TrackTable::Table(std::shared_ptr<Database> db) : db(db), name("track")
{
    std::string query =         
        "CREATE TABLE IF NOT EXISTS track (          \
            id INTEGER PRIMARY KEY AUTOINCREMENT,    \
            type INT,                                \
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
            type,                       \
            modified,                   \
            songName,                   \
            categories,                 \
            bankNo,                     \
            formMask                    \
        )                               \
        VALUES (?, ?, ?, ?, ?, ?, ?)    \
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

    statement.bind_text(entry->path.filename().string());
    statement.bind_int((int)entry->type);
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
    entry->databaseIndex = dbIdx;

    return dbIdx;
}

template<> void TrackTable::create_from_statement(Statement& statement, std::shared_ptr<Track> obj)
{
        obj->databaseIndex = statement.column_int(0);
        obj->type = TrackType(statement.column_int(1));
        obj->path = statement.column_text(2);
        obj->timestamp = statement.column_int64(3);
        obj->name = statement.column_text(4);
        *obj->categories = statement.column_blob(5);
        obj->bankNo = statement.column_int(6);

        std::vector<char> formmask_temp = statement.column_blob(7);
}

template<> void TrackTable::load_entries()
{
    int returnedId = 0;
    Statement statement = select_iter();

    while((returnedId = statement.exec_and_return_id()) > 0)
    {
        std::shared_ptr<Track> entry = std::make_shared<Track>();

        create_from_statement(statement, entry);

        entries.emplace(returnedId, entry);
    }

    return;
}
