#include "table.h"
#include "database.h"

RelationTable::RelationTable(std::shared_ptr<Database> db, std::string name)
{
    this->db = db;
    this->name = name;
}

std::shared_ptr<sqlite3> RelationTable::get_sqlite() { return db->sqlite(); }

int RelationTable::init(bool swap_primary_key)
{
    std::string query;
    if (!swap_primary_key)
    {
        query = std::format(
            "CREATE TABLE IF NOT EXISTS {0}( "
                "{1} INTEGER PRIMARY KEY,    "
                "{2} INTEGER                 "
                ")",
            name, col1, col2
        );
    }
    else
    {
        query = std::format(
            "CREATE TABLE IF NOT EXISTS {0}( "
                "{1} INTEGER,                "
                "{2} INTEGER PRIMARY KEY     "
                ")",
            name, col1, col2
        );
    }

    return db->exec(query);
}

int RelationTable::select(int id)
{
    Statement statement(get_sqlite());

    std::string query = std::format(
        "SELECT * FROM {0} WHERE {1} = {2};",
        name, col1, id
    );

    if (statement.prepare(query))
    {
        return -2;
    }

    if (statement.step() == SQLITE_ROW)
    {
        return statement.column_int(1);
    }
    else
    {
        return -1;
    }
}

Statement RelationTable::select_iter(int id)
{
    Statement statement(get_sqlite());

    std::string query = std::format(("SELECT * FROM {0} WHERE {1}={2};"), name, col1, id, col2);

    statement.prepare(query);
    return statement;
}

int RelationTable::init()
{
    return init(false);
}

int RelationTable::insert(int id_1, int id_2)
{
    std::string query = std::format(
        "INSERT INTO {0} ({1}, {2})"
        "VALUES ({3}, {4})"
        "ON CONFLICT ({1}) DO UPDATE SET {2}={4};",
        name, col1, col2, id_1, id_2
    );

    return db->exec(query);
}

int RelationTable::remove(int id) 
{
    Statement statement(get_sqlite());

    std::string query = std::format("DELETE FROM {0} WHERE {1} = {2} RETURNING {3};", name, col1, id, col2);

    if (statement.prepare(query))
    {
        return -2;
    }

    return (statement.exec_and_return_id());
}

Statement RelationTable::remove_iter(int id)
{
    Statement statement(get_sqlite());

    std::string query = std::format(("DELETE FROM {0} WHERE {1}={2} RETURNING {3};"), name, col1, id, col2);

    statement.prepare(query);
    return statement;
}