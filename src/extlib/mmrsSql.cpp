#include "read_mmrs.h"
#include "mmrsSql.h"

#include <tuple>
#include <format>

sqlite3_stmt *statement;

bool _sql_init(const char* dbPath)
{
    bool success = true;

    int rc = sqlite3_open(dbPath, &db);

    return true;
}

bool init_mmrs_cache()
{
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
           formMask BLOB                            \
           );",
        nullptr,
        nullptr,
        &sqlErrMsg
    );

    SQL_ERR_CHECK("Error initializing MMRS table", "");

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

    SQL_ERR_CHECK("Error initializing Zseq table", "");

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

    SQL_ERR_CHECK("Error initializing MMRS Relation table", "");

        rc = sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS zsound_to_mmrs (        "
           "zsound_id INTEGER PRIMARY KEY,                     "
           "mmrs_id INTEGER                                 "
        ");",
        nullptr,
        nullptr,
        &sqlErrMsg
    );

    SQL_ERR_CHECK("Error initializing Zsound-to-MMRS table", "");

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

    SQL_ERR_CHECK("Error initializing Zbank table", "");

    // TODO:
        // Zsound table
    rc = sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS zsound (                "
           "id INTEGER PRIMARY KEY AUTOINCREMENT,           "
           "size INTEGER,                                   "
           "foreignKey INTEGER,                             "
           "data BLOB                                       "
        ");",
        nullptr,
        nullptr,
        &sqlErrMsg
    );

    SQL_ERR_CHECK("Error initializing Zsound table", "");
    
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

int _count_zsound(int mmrsId)
{
    sqlite3_reset(statement);
    const char* query = "SELECT COUNT(*) FROM zsound_to_mmrs WHERE mmrs_id=?";
    int rc;

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, mmrsId);
    }
    SQL_ERR_CHECK("Error preparing Zsound count statement", "");

    rc = sqlite3_step(statement);
    if (rc == SQLITE_ROW)
    {
        return sqlite3_column_int(statement, 0);
    }
    else
    {
        mmrs_util::error() << "Could not retrieve Zsound count for MMRS " << mmrsId << ": " << sqlite3_errmsg(db) << std::endl;
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

    // Check for matching entry
    const char* query = "SELECT * FROM mmrs WHERE filename=? AND modified=?;";
    sqlite3_reset(statement);

    int rc;

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_text(statement, 1, filepath, strlen(filepath), SQLITE_STATIC);
        sqlite3_bind_int64(statement, 2, timestamp);
    }
    SQL_ERR_CHECK("Error preparing MMRS SELECT statement", "");


    rc = sqlite3_step(statement);

    if (rc == SQLITE_ROW)
    {
        mmrs_util::debug() << "Found matching entry for " << filepath << " with timestamp " << timestamp << ", skipping!" << std::endl;
        return true;
    }
    else if (rc == SQLITE_DONE)
    {
        mmrs_util::debug() << "No matching files found for " << filepath << " with timestamp " << timestamp <<  ", adding to database!" << std::endl;
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
        sqlite3_bind_blob(statement, 6, mmrs.formmask, sizeof(u16) * 17, SQLITE_STATIC);

        sqlite3_bind_int64(statement, 7, timestamp);
        sqlite3_bind_text(statement, 8, mmrs.songName, strlen(mmrs.songName), SQLITE_STATIC);
        sqlite3_bind_blob(statement, 9, mmrs.categories, sizeof(bool) * 256, SQLITE_STATIC);
        sqlite3_bind_int(statement, 10, mmrs.bankNo);
        sqlite3_bind_blob(statement, 11, mmrs.formmask, sizeof(u16) * 17, SQLITE_STATIC);
    }

    SQL_ERR_CHECK("Error preparing MMRS UPSERT statement", "");

    rc = sqlite3_step(statement);
    int mmrsId = -1;

    if (rc == SQLITE_ROW)
    {
        mmrsId = sqlite3_column_int(statement, 0);
    }
    else if (rc == SQLITE_DONE)
    {

    }
    else SQL_ERR_CHECK("Error in MMRS UPSERT execution", "");
    
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
        zseqId = sqlite3_column_int(statement, 0);
    }
    else if (rc == SQLITE_DONE)
    {

    }
    else SQL_ERR_CHECK("Error in Zseq UPSERT execution", "");
    
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

    }
    else SQL_ERR_CHECK("Error in MMRS Relation table UPSERT", "");

    return mmrsId;
}

int insert_zbank(Zbank zbank, int mmrsId)
{
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

    SQL_ERR_CHECK("Error preparing Zbank UPSERT statement", "");

    rc = sqlite3_step(statement);
    int zbankId = -1;

    if (rc == SQLITE_ROW)
    {
        zbankId = sqlite3_column_int(statement, 0);
    }
    else if (rc == SQLITE_DONE)
    {
        mmrs_util::warning() << "Zbank UPSERT with RETURNING clause completed successfully but did not return a row (?!)" << std::endl;
    }
    else SQL_ERR_CHECK("Error in Zbank UPSERT execution", "");
    
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

    SQL_ERR_CHECK("Error preparing MMRS Relation UPDATE statement", "");

    rc = sqlite3_step(statement);

    if (rc == SQLITE_DONE)
    {
        return zbankId;
    }
    else SQL_ERR_CHECK("Error in MMRS Relation UPDATE execution", "");
    
    if (zbankId <= 0)
    {
        mmrs_util::error() << "Error: Zbank UPSERT returned " << zbankId << std::endl;
        return -1;
    }
}

bool insert_zsound(Zsound zsound, int mmrsId)
{
    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Called insert_zsound!";
    mmrs_util::debug() << END_PARA;
    sqlite3_reset(statement);

    int rc = 0;

    // Now upsert the Zbank, and return the ID.
    const char* query = 
        "INSERT INTO zsound (           \
            size,                       \
            foreignKey,                 \
            data                        \
        )                               \
        VALUES (?, ?, ?)                \
        RETURNING id                    \
        ";

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, zsound.size);
        sqlite3_bind_int(statement, 2, zsound.sampleAddr);
        sqlite3_bind_blob(statement, 3, zsound.data, zsound.size, SQLITE_STATIC);
    }

    SQL_ERR_CHECK("Error preparing Zsound INSERT statement", "");

    mmrs_util::debug() << "sampleAddr: " << zsound.sampleAddr << std::endl;

    rc = sqlite3_step(statement);
    int zsoundId = -1;

    if (rc == SQLITE_ROW)
    {
        zsoundId = sqlite3_column_int(statement, 0);
    }
    else if (rc == SQLITE_DONE)
    {

    }
    else SQL_ERR_CHECK("Error in Zsound INSERT execution", "");
    
    if (zsoundId <= 0)
    {
        mmrs_util::error() << "Error: Zsound UPSERT returned " << zsoundId << std::endl;
        return -1;
    }

    sqlite3_reset(statement);

    rc = 0;

    query = 
        "INSERT INTO zsound_to_mmrs  (  \
            zsound_id,                  \
            mmrs_id                     \
        )                               \
        VALUES (?, ?)                   \
        ON CONFLICT (zsound_id) DO      \
        UPDATE SET                      \
            zsound_id=?,                \
            mmrs_id=?;                  \
        ";

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, zsoundId);
        sqlite3_bind_int(statement, 2, mmrsId);
    }

    SQL_ERR_CHECK("Error preparing Zbank-to-Zsound UPSERT statement", "");

    rc = sqlite3_step(statement);

    if (rc == SQLITE_DONE)
    {
        return zsoundId;
    }
    else SQL_ERR_CHECK("Error in Zbank-to-Zsound UPSERT execution", "Zbank-to-Zsound UPSERT with RETURNING clause completed successfully but did not return a row (?!)");
    
    if (zsoundId <= 0)
    {
        mmrs_util::error() << "Error: Zbank-to-Zsound UPSERT returned " << zsoundId << std::endl;
        return -1;
    }
}

bool _load_mmrs_table(MMRS* allMmrs)
{
    sqlite3_reset(statement);
    sqlite3_stmt *substatement;
    const char* query = "SELECT * FROM mmrs;";

    int rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr);

    int i = 0;

    while ((rc = sqlite3_step(statement)) == SQLITE_ROW)
    {
        // mmrs_util::debug() << sqlite3_column_int(statement, 0) << ", " << sqlite3_column_text(statement, 3) << ", timestamp " << sqlite3_column_int64(statement, 2) << std::endl;
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
            allMmrs[i].categories[c ^ 3] = cats[c];
        }

        // Audiobank number
        allMmrs[i].bankNo = sqlite3_column_int(statement, 5);

        // Form mask
        unsigned char* formmask = (unsigned char*)sqlite3_column_blob(statement, 6);

        for (int j = 0; j < 36; j++)
        {
            ((unsigned char*)&(allMmrs[i].formmask))[j] = formmask[j ^ 2];
        }

        // Zseq ID
        const char* subquery = "SELECT * FROM mmrs_relation WHERE mmrs_id=?";
        if (sqlite3_prepare_v2(db, subquery, -1, &substatement, nullptr) == SQLITE_OK)
        {
                sqlite3_bind_int(substatement, 1, allMmrs[i].id);
        }

        if ((rc =sqlite3_step(substatement)) == SQLITE_ROW)
        {
            allMmrs[i].zseqId = sqlite3_column_int(substatement, 1);
            allMmrs[i].bankInfoId = sqlite3_column_int(substatement, 2);
        }
        i++;
    }

    sqlite3_finalize(substatement);

    if (rc == SQLITE_DONE)
    {
        if (i == 0)
        {
            mmrs_util::error() << "Error: MMRS table is empty." << std::endl;

            return false;
        }
        else
        {
            // mmrs_util::debug() << std::endl << "Reached end of table." << std::endl;
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
        zseqAddr->size = sqlite3_column_int(statement, 1);
        const unsigned char* zseqData = (const unsigned char*)sqlite3_column_blob(statement, 2);
        
        for (int i = 0; i < zseqAddr->size; i++)
        {
            zseqAddr->data[i ^ 3] = zseqData[i];
        }
        return true;
    }
    else if (rc == SQLITE_DONE)
    {
            return false;
    }
    else
    {
        return false;
    }
}

bool _load_zbank(Zbank* zbankAddr, int zbankId)
{    
    sqlite3_reset(statement);

    const char* query = "SELECT * FROM zbank WHERE id=?";
    int rc;

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, zbankId);
    }
    else
    {
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

        for (int i = 0; i < zbankAddr->bankSize; i++)
        {
            zbankAddr->bankData[i ^ 3] = bankDataBuff[i];
        }

        return true;
    }
    else if (rc == SQLITE_DONE)
    {
        mmrs_util::error() << "Error: Could not find zbank with ID " << zbankId;

        return false;
    }
    else
    {
        mmrs_util::error() << "Error loading Zbank from db: " << sqlite3_errmsg(db);
        return false;
    }
}



bool _load_zsound(Zsound* zsoundAddr, int mmrsId)
{
    mmrs_util::debug() << START_PARA;
    mmrs_util::debug() << "Called load_zsound";
    mmrs_util::debug() << END_PARA;

    sqlite3_reset(statement);

    int rc;

    const char *query = "SELECT * FROM zsound LEFT JOIN zsound_to_mmrs ON zsound_id = zsound.id WHERE mmrs_id=?";
    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, mmrsId);
    }

    
    int i = 0;

    while ((rc = sqlite3_step(statement)) == SQLITE_ROW)
    {
        mmrs_util::debug() << "!!" << std::endl;
        zsoundAddr[i].size = sqlite3_column_int(statement, 1);
        zsoundAddr[i].sampleAddr = sqlite3_column_int(statement, 2);
        const unsigned char* zsoundData = (const unsigned char*)sqlite3_column_blob(statement, 3);

        mmrs_util::debug() << "Data for zsound " << i << " begins ";
        
        for (int j = 0; j < zsoundAddr[i].size; j++)
        {
            zsoundAddr[i].data[j ^ 3] = zsoundData[j];
            if (j < 16)
            {
                mmrs_util::debug() << std::format("{:02x} ", zsoundAddr[i].data[j ^ 3]);
            }
        }

        mmrs_util::debug() << std::endl;

        i++;
        mmrs_util::debug() << "?!";
    }
    mmrs_util::debug() << "Successfully copied " << i << " custom sounds for MMRS with id " << mmrsId << "!" << std::endl;
    mmrs_util::debug() << END_PARA;

    if (rc == SQLITE_DONE && i == 0)
    {
            mmrs_util::error() << "No matching Zseq found for Zsound ID " << zsoundAddr << std::endl;
            mmrs_util::debug() << END_PARA;
            return false;
    }
    else if (i == 0)
    {
        mmrs_util::error() << "Error fetching Zsound: " << sqlite3_errmsg(db) << std::endl;
        mmrs_util::debug() << END_PARA;
        return false;
    }
    else
    {
        return true;
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
        // mmrs_util::debug() << "Finished selecting filenames" << std::endl;
    }
    else 
    {
        mmrs_util::error() << "Error retrieving MMRS filenames: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}

bool remove_mmrs(int mmrsId)
{
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
        mmrs_util::info() << "Successfully removed MMRS with ID " << mmrsId << " from the database." << std::endl;
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
        zseqId = sqlite3_column_int(statement, 0);
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
    }
    else
    {
        mmrs_util::error() << "Error deleting Zseq with ID " << zseqId << ": " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_reset(statement);

    query = "DELETE FROM zsound_to_mmrs WHERE mmrs_id=? RETURNING zsound_id";

    rc = 0;
    int zsoundId = 0;

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, mmrsId);
    }

    rc = sqlite3_step(statement);
    if (rc == SQLITE_ROW)
    {
        zsoundId = sqlite3_column_int(statement, 0);
    }
    else
    {
        mmrs_util::error() << "Error deleting Zsound-to-MMRS with ID " << mmrsId << ": " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_reset(statement);

    query = "DELETE FROM zsound WHERE id=? RETURNING id";

    rc = 0;

    if ((rc = sqlite3_prepare_v2(db, query, -1, &statement, nullptr)) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, zsoundId);
    }

    rc = sqlite3_step(statement);
    if (rc == SQLITE_ROW)
    {
        zsoundId = sqlite3_column_int(statement, 0);
    }
    else
    {
        mmrs_util::error() << "Error deleting Zsound with ID " << zsoundId << ": " << sqlite3_errmsg(db) << std::endl;
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
    else
    {
        rc = sqlite3_close(db);
        if (rc != SQLITE_OK)
        {
            mmrs_util::error() << "Error closing DB: " << sqlite3_errmsg(db) << std::endl;
            success = false;
        }
    }
    return success;
}