#include "read_mmrs.h"
#include "mmrsSql.h"

#include <tuple>

sqlite3_stmt *statement;

bool _sql_init(const char* dbPath)
{
    bool success = true;

    printf("%s (I am in the um)\n", dbPath);
    printf("%p (I am in the um)\n", &db);

    int rc = sqlite3_open(dbPath, &db);
    printf("K M S\n");
    // SQL_ERR_CHECK("Error opening db", "Successfully opened db!");

    return true;
}

bool init_mmrs_cache()
{

    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Called init_mmrs_cache";
    mmrs_util::debug() << END_PARA;

    char *sqlErrMsg;
    int rc;

    // Initialize all tables    
    rc = sqlite3_exec(
        db, 
        "CREATE TABLE IF NOT EXISTS mmrs (          \
           id INTEGER PRIMARY KEY AUTOINCREMENT,    \
           filename TEXT UNIQUE,                    \
           modified INTEGER,                        \
           songName TEXT,                           \
           categories BLOB,                         \
           bankNo INTEGER,                          \
           formMask INTEGER                         \
           );",
        nullptr,
        nullptr,
        &sqlErrMsg
    );

    SQL_ERR_CHECK("Error initializing MMRS table", "Successfully initialized MMRS table!");

    rc = sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS zseq (           "
           "id INTEGER PRIMARY KEY AUTOINCREMENT,    "
           "size INTEGER,                            "
           "data BLOB                                "
        ");",
        nullptr,
        nullptr,
        &sqlErrMsg
    );

    SQL_ERR_CHECK("Error initializing Zseq table", "Successfully initialized Zseq table!");

    rc = sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS mmrs_relation (         "
           "mmrs_id INTEGER PRIMARY KEY,                    "
           "zseq_id INTEGER,                                "
           "zbank_id INTEGER                                "
        ");",
        nullptr,
        nullptr,
        &sqlErrMsg
    );

    SQL_ERR_CHECK("Error initializing MMRS Relation table", "Successfully initialized MMRS Relation table!");

        rc = sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS zsound_to_mmrs (        "
           "zsound INTEGER PRIMARY KEY,                     "
           "mmrs_id INTEGER                                 "
        ");",
        nullptr,
        nullptr,
        &sqlErrMsg
    );

    SQL_ERR_CHECK("Error initializing Zsound-to-MMRS table", "Successfully initialized Zsound-to-MMRS table!");

    rc = sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS zbank (                 "
           "id INTEGER PRIMARY KEY AUTOINCREMENT,           "
           "headerSize INTEGER,                             "
           "header BLOB,                                    "
           "dataSize INTEGER,                               "
           "data BLOB                                       "
        ");",
        nullptr,
        nullptr,
        &sqlErrMsg
    );

    SQL_ERR_CHECK("Error initializing Zbank table", "Successfully initialized Zbank table!");

    // TODO:
        // Zsound table
    
    return true;
}

int count_mmrs()
{
    const char* query = "SELECT COUNT(*) FROM mmrs";
    sqlite3_reset(statement);

    int rc;
    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        rc = sqlite3_step(statement);
        if (rc == SQLITE_ROW)
        {
            return sqlite3_column_int(statement, 0);
        }
        else
        {
            mmrs_util::error() << "Could not retrieve MMRS count: " << sqlite3_errmsg(db) << std::endl;
            return -1;
        }
    }
    else
    {
        mmrs_util::error() << "Error preparing count statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
}

bool check_mmrs_exists(fs::directory_entry file)
{
    char *sqlErrMsg;
    int sqlErrCode;

    std::string fPathStrTemp = file.path().filename().string();
    const char *filepath = fPathStrTemp.c_str();
    uint64_t timestamp = file.last_write_time().time_since_epoch().count();

    mmrs_util::debug() << "Filepath is " << filepath << std::endl;

    // Check for matching entry
    const char* query = "SELECT * FROM mmrs WHERE filename=? AND modified=?;";
    sqlite3_reset(statement);

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

    return false;
}

int insert_mmrs(MMRS mmrs, Zseq zseq, fs::directory_entry file)
{
    char *sqlErrMsg;
    int sqlErrCode;

    std::string filepathStr = file.path().filename().string();
    const char *filepath = filepathStr.c_str();
    uint64_t timestamp = file.last_write_time().time_since_epoch().count();

    sqlite3_reset(statement);

    sqlite3_prepare(db, "SELECT * FROM mmrs", -1, &statement, nullptr);
    if (sqlite3_step(statement) == SQLITE_DONE)
    {
        for (int i = 0; i < 6; i++)
        {
            mmrs_util::debug() << sqlite3_column_name(statement, i) << " ";
        }
        mmrs_util::debug() << std::endl;
    }

    int rc = 0;

    // Now upsert the MMRS, and return the ID.
    const char* query = 
        "INSERT INTO mmrs (             \
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

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_text(statement, 1, filepath, strlen(filepath), SQLITE_STATIC);
        sqlite3_bind_int64(statement, 2, timestamp);
        sqlite3_bind_text(statement, 3, mmrs.songName, strlen(mmrs.songName), SQLITE_STATIC);
        sqlite3_bind_blob(statement, 4, mmrs.categories, sizeof(bool) * 256, SQLITE_STATIC);
        sqlite3_bind_int(statement, 5, mmrs.bankNo);
        sqlite3_bind_int(statement, 6, mmrs.formmask);

        sqlite3_bind_int64(statement, 7, timestamp);
        sqlite3_bind_text(statement, 8, mmrs.songName, strlen(mmrs.songName), SQLITE_STATIC);
        sqlite3_bind_blob(statement, 9, mmrs.categories, sizeof(bool) * 256, SQLITE_STATIC);
        sqlite3_bind_int(statement, 10, mmrs.bankNo);
        sqlite3_bind_int(statement, 11, mmrs.formmask);
    }

    SQL_ERR_CHECK("Error preparing MMRS UPSERT statement", "Successfully prepared MMRS UPSERT statement!");

    rc = sqlite3_step(statement);
    int mmrsId = -1;

    if (rc == SQLITE_ROW)
    {
        mmrsId = sqlite3_column_int(statement, 0);
    }
    else if (rc == SQLITE_DONE)
    {
        mmrs_util::debug() << "Successfully performed MMRS UPSERT statement!" << std::endl;
    }
    else SQL_ERR_CHECK("Error in MMRS UPSERT execution", "MMRS UPSERT with RETURNING clause completed successfully but did not return a row (?!)");
    
    if (mmrsId <= 0)
    {
        mmrs_util::error() << "Error: MMRS UPSERT returned " << mmrsId << std::endl;
        return -1;
    }

    sqlite3_reset(statement);

    int zseqId = -1;

    // Now do the Zseq table
    query = 
        "INSERT INTO zseq (             \
            size,                       \
            data                        \
        )                               \
        VALUES (?, ?)                   \
        ON CONFLICT (id) DO             \
        UPDATE SET                      \
            size=?,                     \
            data=?                      \
        RETURNING id;                   \
        ";
    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, zseq.size);
        sqlite3_bind_blob(statement, 2, zseq.data, sizeof(unsigned char) * zseq.size, SQLITE_STATIC);
        sqlite3_bind_int(statement, 3, zseq.size);
        sqlite3_bind_blob(statement, 4, zseq.data, sizeof(unsigned char) * zseq.size, SQLITE_STATIC);
    }

    rc = sqlite3_step(statement);

    if (rc == SQLITE_ROW)
    {
        mmrs_util::debug() << "Successfully performed Zseq UPSERT statement!" << std::endl;
        zseqId = sqlite3_column_int(statement, 0);
    }
    else if (rc == SQLITE_DONE)
    {
        mmrs_util::debug() << "Zseq UPSERT with RETURNING clause returned SQLITE_OK (?!)" << std::endl;
    }
    else SQL_ERR_CHECK("Error in Zseq UPSERT execution", "UPSERT clause executed successfully!");
    
    if (zseqId <= 0)
    {
        mmrs_util::error() << "Error: Zseq UPSERT returned " << zseqId << std::endl;
        return -1;
    }

    sqlite3_reset(statement);

    // Relat table
    query = 
        "INSERT INTO mmrs_relation (    \
            mmrs_id,                    \
            zseq_id,                    \
            zbank_id                    \
        )                               \
        VALUES (?, ?, ?)                \
        ON CONFLICT (mmrs_id) DO        \
        UPDATE SET                      \
            zseq_id=?,                  \
            zbank_id=?;                 \
        ";
    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, mmrsId);
        sqlite3_bind_int(statement, 2, zseqId);
        sqlite3_bind_int(statement, 3, -1); //PLACEHOLDER
        sqlite3_bind_int(statement, 4, zseqId);
        sqlite3_bind_int(statement, 5, -1); //PLACEHOLDER
    }

    rc = sqlite3_step(statement);
    if (rc == SQLITE_DONE)
    {
        mmrs_util::debug() << "MMRS Relation returned SQLITE_DONE for some reason but seems ok otherwise" << std::endl;
    }
    else SQL_ERR_CHECK("Error in MMRS Relation table UPSERT", "MMRS Relation table UPSERTed successfully!");

    return mmrsId;
}

int insert_zbank(Zbank zbank, int mmrsId)
{
    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Called insert_zbank!";
    mmrs_util::debug() << END_PARA;
    sqlite3_reset(statement);

    int rc = 0;

    // Now upsert the Zbank, and return the ID.
    const char* query = 
        "INSERT INTO zbank (            \
            headerSize,                 \
            header,                     \
            dataSize,                   \
            data                        \
        )                               \
        VALUES (?, ?, ?, ?)             \
        RETURNING id                    \
        ";

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, zbank.metaSize);
        sqlite3_bind_blob(statement, 2, zbank.metaData, zbank.metaSize, SQLITE_STATIC);
        sqlite3_bind_int(statement, 3, zbank.bankSize);
        sqlite3_bind_blob(statement, 4, zbank.bankData, zbank.bankSize, SQLITE_STATIC);
    }

    SQL_ERR_CHECK("Error preparing Zbank UPSERT statement", "Successfully prepared Zbank UPSERT statement!");

    rc = sqlite3_step(statement);
    int zbankId = -1;

    if (rc == SQLITE_ROW)
    {
        zbankId = sqlite3_column_int(statement, 0);
        mmrs_util::debug() << "Successfully performed Zbank UPSERT statement!" << std::endl;
    }
    else if (rc == SQLITE_DONE)
    {
        mmrs_util::debug() << "Zbank UPSERT with RETURNING clause completed successfully but did not return a row (?!)" << std::endl;
    }
    else SQL_ERR_CHECK("Error in Zbank UPSERT execution", "Zbank UPSERT with RETURNING clause completed successfully but returned SQLITE_OK (?!)");
    
    if (zbankId <= 0)
    {
        mmrs_util::error() << "Error: Zbank UPSERT returned " << zbankId << std::endl;
        return -1;
    }

    sqlite3_reset(statement);

    rc = 0;

    query = 
        "UPDATE mmrs_relation           \
            SET zbank_id=?              \
            WHERE mmrs_id=?             \
        ";

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, zbankId);
        sqlite3_bind_int(statement, 2, mmrsId);
    }

    SQL_ERR_CHECK("Error preparing MMRS Relation UPDATE statement", "Successfully prepared MMRS Relation UPDATE statement!");

    rc = sqlite3_step(statement);

    if (rc == SQLITE_DONE)
    {
        return zbankId;
    }
    else SQL_ERR_CHECK("Error in MMRS Relation UPDATE execution", "MMRS UPSERT with RETURNING clause completed successfully but did not return a row (?!)");
    
    if (zbankId <= 0)
    {
        mmrs_util::error() << "Error: Zbank UPSERT returned " << zbankId << std::endl;
        return -1;
    }
}

bool _load_mmrs_table(MMRS* allMmrs)
{

    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "MMRS Table on load call: ";
    mmrs_util::debug() << END_PARA;

    sqlite3_reset(statement);
    sqlite3_stmt *substatement;
    const char* query = "SELECT * FROM mmrs;";

    int rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr);

    int i = 0;

    while ((rc = sqlite3_step(statement)) == SQLITE_ROW)
    {
        mmrs_util::debug() << sqlite3_column_int(statement, 0) << ", " << sqlite3_column_text(statement, 3) << ", timestamp " << sqlite3_column_int64(statement, 2) << std::endl;
        // Table index
        allMmrs[i].id = sqlite3_column_int(statement, 0);

        // Song name
        const char* songName = (const char*)sqlite3_column_text(statement, 3);

        for (int n = 0; n < strlen(songName) + 4 - (strlen(songName) % 4); n++) 
        {
            allMmrs[i].songName[n ^ 3] = songName[n];
        }

        // Categories
        const unsigned char* cats = (const unsigned char*)sqlite3_column_blob(statement, 4);
        
        for (int c = 0; c < 256; c++)
        {
            allMmrs[i].categories[c] = cats[c];
            // mmrs_util::debug() << c << std::endl;
        }

        // Audiobank number
        allMmrs[i].bankNo = sqlite3_column_int(statement, 5);

        // Form mask
        allMmrs[i].formmask = sqlite3_column_int(statement, 6);

        // Zseq ID
        const char* subquery = "SELECT * FROM mmrs_relation WHERE mmrs_id=?";
        if (sqlite3_prepare_v2(db, subquery, -1, &substatement, nullptr) == SQLITE_OK)
        {
                sqlite3_bind_int(substatement, 1, allMmrs[i].id);
        }

        if ((rc =sqlite3_step(substatement)) == SQLITE_ROW)
        {
            allMmrs[i].zseqId = sqlite3_column_int(substatement, 1);
            mmrs_util::debug() << "ZseqId: " << allMmrs[i].zseqId << std::endl;
            allMmrs[i].bankInfoId = sqlite3_column_int(substatement, 2);
            mmrs_util::debug() << "BankInfoId: " << allMmrs[i].bankInfoId << std::endl;
        }
        i++;
    }

    sqlite3_finalize(substatement);

    if (rc == SQLITE_DONE)
    {
        if (i == 0)
        {
            mmrs_util::error() << "Error: MMRS table is empty." << std::endl;
            mmrs_util:: debug() << END_PARA;

            return false;
        }
        else
        {
            mmrs_util::debug() << std::endl << "Reached end of table." << std::endl;
        }
    }
    else
    {
        SQL_ERR_CHECK("Error fetching MMRS table", "MMRS table fetch returned SQLITE_OK... If you see this, something has gone terribly wrong.");
        return false;
    }

    return true;
}

bool _load_zseq(Zseq* zseqAddr, int zseqId)
{

    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Called load_zseq";
    mmrs_util::debug() << END_PARA;

    sqlite3_reset(statement);

    int rc;

    const char *query = "SELECT * FROM zseq WHERE id=?";
    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, zseqId);
    }

    rc = sqlite3_step(statement);

    if (rc == SQLITE_ROW)
    {
        mmrs_util::debug() << "The" << std::endl;
        zseqAddr->size = sqlite3_column_int(statement, 1);
        const unsigned char* zseqData = (const unsigned char*)sqlite3_column_blob(statement, 2);
        
        for (int i = 0; i < zseqAddr->size; i++)
        {
            zseqAddr->data[i ^ 3] = zseqData[i];
        }
        mmrs_util::debug() << "Successfully copied Zseq with ID " << zseqId << "!" << std::endl;
        mmrs_util::debug() << END_PARA;
        return true;
    }
    else if (rc == SQLITE_DONE)
    {
            mmrs_util::error() << "No matching Zseq found for Zseq ID " << zseqId << std::endl;
            mmrs_util::debug() << END_PARA;
            return false;
    }
    else
    {
        mmrs_util::error() << "Error fetching Zseq: " << sqlite3_errmsg(db) << std::endl;
        mmrs_util::debug() << END_PARA;
        return false;
    }
}

bool _load_zbank(Zbank* zbankAddr, int zbankId)
{
    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Called load_zbank";
    mmrs_util::debug() << END_PARA;
    
    sqlite3_reset(statement);

    const char* query = "SELECT * FROM zbank WHERE id=?";
    int rc;

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, zbankId);
    }
    else
    {
        mmrs_util::debug() << "Error preparing Zbank SELECT statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(statement);

    if (rc == SQLITE_ROW)
    {
        // Bankmeta (header)
        zbankAddr->metaSize = sqlite3_column_int(statement, 1);
        const unsigned char* bankMetaBuff = (const unsigned char*)sqlite3_column_blob(statement, 2);

        for (int i = 0; i < zbankAddr->metaSize; i++)
        {
            zbankAddr->metaData[i ^ 3] = bankMetaBuff[i];
        }

        // Bank data
        zbankAddr->bankSize = sqlite3_column_int(statement, 3);
        const unsigned char* bankDataBuff = (const unsigned char*)sqlite3_column_blob(statement, 4);

        for (int i = 0; i < 32; i++)
        {
            zbankAddr->bankData[i ^ 3] = bankDataBuff[i];
            // mmrs_util::debug() << std::hex << (int)bankDataBuff[i] << " ";
        }
        // mmrs_util::debug() << std::endl;

        return true;
    }
    else if (rc == SQLITE_DONE)
    {
        mmrs_util::error() << "Error: Could not find zbank with ID " << zbankId;
        mmrs_util::debug() << END_PARA;
        return false;
    }
    else
    {
        mmrs_util::error() << "Error loading Zbank from db: " << sqlite3_errmsg(db);
        mmrs_util::debug() << END_PARA;
        return false;
    }
}

bool retrieve_filenames(int* ids, std::string* filenames)
{
    sqlite3_reset(statement);

    const char *query = "SELECT * FROM mmrs";

    int rc = 0;
    int i = 0;

    rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr);

    while ((rc = sqlite3_step(statement)) == SQLITE_ROW)
    {
        const unsigned char* fname = sqlite3_column_text(statement, 1);
        std::string fnameStr((const char*)fname);
        int id = sqlite3_column_int(statement, 0);

        filenames[i] = fnameStr;
        ids[i] = id;
        i++;
    }
    if (rc == SQLITE_DONE)
    {
        mmrs_util::debug() << "Finished selecting filenames" << std::endl;
    }
    else 
    {
        mmrs_util::debug() << "Error selecting filenames:" << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}

bool remove_mmrs(int mmrsId)
{

    mmrs_util::debug() << START_PARA;
    sqlite3_reset(statement);

    const char *query = "DELETE FROM mmrs WHERE id=? RETURNING id";

    int rc = 0;

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, mmrsId);
    }

    rc = sqlite3_step(statement);
    if (rc == SQLITE_ROW)
    {
        mmrsId = sqlite3_column_int(statement, 0);
        mmrs_util::debug() << "Successfully deleted MMRS with ID " << mmrsId << std::endl;
    }
    else
    {
        mmrs_util::error() << "Error deleting MMRS with ID " << mmrsId << ": " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_reset(statement);

    query = "DELETE FROM mmrs_relation WHERE mmrs_id=? RETURNING zseq_id";

    rc = 0;
    int zseqId = 0;

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, mmrsId);
    }

    rc = sqlite3_step(statement);
    if (rc == SQLITE_ROW)
    {
        mmrs_util::debug() << "Successfully deleted MMRS relation with ID " << mmrsId << std::endl;
        zseqId = sqlite3_column_int(statement, 0);
        mmrs_util::debug() << "Zseq id is " << zseqId << std::endl;
    }
    else
    {
        mmrs_util::error() << "Error deleting MMRS relation with ID " << mmrsId << ": " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_reset(statement);

    query = "DELETE FROM zseq WHERE id=? RETURNING id";

    rc = 0;

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, zseqId);
    }

    rc = sqlite3_step(statement);
    if (rc == SQLITE_ROW)
    {
        zseqId = sqlite3_column_int(statement, 0);
        mmrs_util::debug() << "Successfully deleted Zseq with ID " << zseqId << std::endl;
    }
    else
    {
        mmrs_util::error() << "Error deleting Zseq with ID " << zseqId << ": " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_reset(statement);

    return true;
}

bool _sql_teardown()
{
    bool success = true;

    int rc = sqlite3_finalize(statement);
    if (rc != SQLITE_OK)
    {
        mmrs_util::error() << "Error finalizing statement: " << sqlite3_errmsg(db) << std::endl;
        success = false;
    }
    
    rc = sqlite3_close(db);
    if (rc != SQLITE_OK)
    {
        mmrs_util::error() << "Error closing DB: " << sqlite3_errmsg(db) << std::endl;
        success = false;
    }

    return success;
}