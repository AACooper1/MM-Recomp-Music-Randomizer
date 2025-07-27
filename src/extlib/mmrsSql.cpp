#include "read_mmrs.h"
#include "mmrsSql.h"

bool init_mmrs_cache(const char* dbPath)
{
    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Called init_mmrs_cache with dbPath: " << dbPath;
    mmrs_util::debug() << END_PARA;

    char *sqlErrMsg;
    int rc;

    rc = sqlite3_open(dbPath, &db);

    SQL_ERR_CHECK("Error connecting to MMRS database", "Successfully connected to MMRS database!");

    // Initialize all tables    
    rc = sqlite3_exec(
        db, 
        "CREATE TABLE IF NOT EXISTS mmrs (          \
           id INTEGER PRIMARY KEY AUTOINCREMENT,    \
           filename TEXT UNIQUE,                    \
           modified INTEGER,                        \
           songName TEXT,                           \
           categories TEXT                          \
           );",
        nullptr,
        nullptr,
        &sqlErrMsg
    );

    SQL_ERR_CHECK("Error initializing MMRS database", "Successfully initialized MMRS database!");

    rc = sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS zseq (           "
           "id INTEGER PRIMARY KEY,                  "
           "size INTEGER,                            "
           "data BLOB,                               "
           "bankNo INTEGER,                          "
           "formMask INTEGER                         "
        ");",
        nullptr,
        nullptr,
        &sqlErrMsg
    );

    SQL_ERR_CHECK("Error initializing Zseq database", "Successfully initialized MMRS database!");

    // TODO:
        // Zsound table
        
        // Zbank table
    return true;
}

bool check_mmrs_exists(fs::directory_entry file)
{
    char *sqlErrMsg;
    int sqlErrCode;

    const char *filepath = file.path().filename().string().c_str();
    uint64_t timestamp = file.last_write_time().time_since_epoch().count();

    mmrs_util::debug() << "Filepath is " << filepath << std::endl;

    // Check for matching entry
    const char* query = "SELECT * FROM mmrs WHERE filename='?' AND modified=?;";
    sqlite3_stmt *statement;

    int rc;

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_text(statement, 1, filepath, strlen(filepath), SQLITE_STATIC);
        sqlite3_bind_int64(statement, 2, timestamp);
    }
    SQL_ERR_CHECK("Error preparing MMRS SELECT statement", "Successfully prepared MMRS SELECT statement!");


    rc = sqlite3_step(statement);

    if (rc == SQLITE_ROW)
    {
        mmrs_util::debug() << "Found matching entry for " << filepath << " with timestamp " << timestamp << ", skipping!" << std::endl;
        return true;
    }
    else if (rc == SQLITE_DONE)
    {
        mmrs_util::debug() << "No matching files found for " << filepath << " with timestamp " << timestamp <<  ", continuing!" << std::endl;
        return false;
    }
    else SQL_ERR_CHECK("Error in SELECT statement", "SELECT statement returned SQLITE_OK for some reason.......?");

    sqlite3_finalize(statement);
}

bool insert_mmrs(MMRS mmrs, fs::directory_entry file)
{
    char *sqlErrMsg;
    int sqlErrCode;

    const char *filepath = file.path().filename().string().c_str();
    uint64_t timestamp = file.last_write_time().time_since_epoch().count();

    sqlite3_stmt *statement;

    int rc = 0;

    // Now upsert the MMRS, and return the ID.
    const char* query = 
        "INSERT INTO mmrs (             \
            filename,                   \
            modified,                   \
            songName,                   \
            categories                  \
        )                               \
        VALUES (?, ?, ?, ?)             \
        ON CONFLICT (filename) DO       \
        UPDATE SET                      \
            modified=?,                 \
            songName='?',               \
            categories=?                \
        RETURNING id;";

    int id;
    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_text(statement, 1, filepath, strlen(filepath), SQLITE_STATIC);
        sqlite3_bind_int64(statement, 2, timestamp);
        sqlite3_bind_text(statement, 3, mmrs.songName, strlen(mmrs.songName), SQLITE_STATIC);
        sqlite3_bind_blob(statement, 4, mmrs.categories, sizeof(bool) * 256, SQLITE_STATIC);

        sqlite3_bind_int64(statement, 5, timestamp);
        sqlite3_bind_text(statement, 6, mmrs.songName, strlen(mmrs.songName), SQLITE_STATIC);
        sqlite3_bind_blob(statement, 7, mmrs.categories, sizeof(bool) * 256, SQLITE_STATIC);
    }

    SQL_ERR_CHECK("Error preparing MMRS UPSERT statement", "Successfully prepared MMRS UPSERT statement!");

    rc = sqlite3_step(statement);
    int rowId = -1;

    if (rc == SQLITE_ROW)
    {
        rowId = sqlite3_column_int(statement, 0);
    }
    else if (rc == SQLITE_DONE)
    {
        mmrs_util::debug() << "Successfully performed MMRS UPSERT statement!" << std::endl;
    }
    else SQL_ERR_CHECK("Error in MMRS UPSERT execution", "MMRS UPSERT with RETURNING clause completed successfully but did not return a row (?!)");
    
    if (rowId <= 0)
    {
        return false;
    }

    // Now do the Zseq table
    query = 
        "INSERT INTO zseq (             \
            id,                         \
            size,                       \
            data,                       \
            formmask                    \
        )                               \
        VALUES (?, ?, ?, ?)             \
        ON CONFLICT (id) DO             \
        UPDATE SET                      \
            size=?,                     \
            data=?,                     \
            formmask=?;                 \
        ";
    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, rowId);
        sqlite3_bind_int(statement, 2, mmrs.zseq.size);
        sqlite3_bind_blob(statement, 3, mmrs.zseq.data, sizeof(unsigned char) * 32768, nullptr);
        sqlite3_bind_blob(statement, 4, mmrs.zseq.formmask, sizeof(bool) * 256, nullptr);
        sqlite3_bind_int(statement, 5, mmrs.zseq.size);
        sqlite3_bind_blob(statement, 6, mmrs.zseq.data, sizeof(unsigned char) * 32768, nullptr);
        sqlite3_bind_blob(statement, 7, mmrs.zseq.formmask, sizeof(bool) * 256, nullptr);
    }

    rc = sqlite3_step(statement);
    rowId = -1;

    if (rc == SQLITE_ROW)
    {
        mmrs_util::debug() << "Zseq UPSERT without RETURNING clause returned a row (?!)";
    }
    else if (rc == SQLITE_DONE)
    {
        mmrs_util::debug() << "Successfully performed Zseq UPSERT statement!" << std::endl;
    }
    else SQL_ERR_CHECK("Error in Zseq UPSERT execution", "UPSERT clause executed successfully!");
    
    if (rowId <= 0)
    {
        return false;
    }

    return true;
}