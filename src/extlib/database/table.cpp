#include "table.h"
#include "database.h"

template<typename T>
std::shared_ptr<sqlite3> Table<T>::get_sqlite() { return db->sqlite(); }

template <typename T>
int Table<T>::exec(std::string query)
{
    return this->db->exec(query);
}

template<typename T>
Statement Table<T>::select_iter()
{
    Statement statement(get_sqlite());

    std::string query = std::format(("SELECT * FROM {0};"), name);

    statement.prepare(query);
    return statement;

}

template<typename T>
Statement Table<T>::select_iter(std::string cols[], int ncol)
{
    Statement statement(get_sqlite());

    std::string query = std::format(("SELECT ("), name);
    for (int i = 0; i < ncol; i++)
    {
        query += cols[i];
        if (i != ncol - 1)
        {
            query += ",";
        }
    }
    query += std::format(") FROM {0};", name);

    statement.prepare(query);
    return statement;

}


template<typename T>
Statement Table<T>::select_iter(std::string query)
{
    Statement statement(get_sqlite());

    std::string prefix = std::format(("SELECT * FROM {0} WHERE "), name);
    query = prefix + query + ";";

    statement.prepare(query);
    return statement;

}

template<typename T>
Statement Table<T>::select_iter(std::string query, std::string cols[], int ncol)
{
    Statement statement(get_sqlite());

    std::string prefix = "SELECT (";
    for (int i = 0; i < ncol; i++)
    {
        prefix += cols[i];
        if (i != ncol - 1)
        {
            prefix += ",";
        }
    }
    prefix += std::format(") FROM {0} WHERE ", name);

    query = prefix + query + ";";

    statement.prepare(query);
    return statement;
}

template<typename T>
Statement Table<T>::select_iter(int id, std::string cols, int ncol)
{
    Statement statement(get_sqlite());

    std::string query = "SELECT (";
    for (int i = 0; i < ncol; i++)
    {
        query += cols[i];
        if (i != ncol - 1)
        {
            query += ",";
        }
    }
    query += std::format(") FROM {0} WHERE id=", name);

    query = query + std::to_string(id) + ";";

    statement.prepare(query);
    return statement;
}

template<typename T>
Statement Table<T>::select_iter(int id)
{
    Statement statement(get_sqlite());

    std::string query = std::format("SELECT * FROM {0} WHERE id={1};", name, id);

    statement.prepare(query);
    return statement;

}

template <typename T>
int Table<T>::remove(std::string query)
{
    Statement statement(get_sqlite());

    std::string prefix = std::format("DELETE FROM {0} WHERE ", name);

    query = prefix + query + " RETURNING id;";

    if (statement.prepare(query))
    {
        return -2;
    }

    return (statement.exec_and_return_id());
}

template <typename T>
int Table<T>::remove(int id)
{
    Statement statement(get_sqlite());

    std::string query = std::format("DELETE FROM {0} WHERE id = {1} RETURNING id;", name, id);

    if (statement.prepare(query))
    {
        return -2;
    }

    return (statement.exec_and_return_id());
}

template<typename T> std::shared_ptr<T> Table<T>::select(std::string query) {}
template<typename T> std::shared_ptr<T> Table<T>::select(std::string query, std::string cols[], int ncol) {}

template<typename T> bool Table<T>::check_exists(int id)
{
    Statement statement(get_sqlite());

    std::string query = std::format("SELECT * FROM {0} WHERE id={1};", name, id);

    statement.prepare(query);
    return (statement.exec_and_return_id() > 0);
}

template<typename T> std::shared_ptr<T> Table<T>::select(int id)
{
    Statement statement(get_sqlite());
    std::string query = std::format("SELECT * FROM {0} WHERE id={1};", name, id);
    statement.prepare(query);

    std::shared_ptr<T> obj = std::make_shared<T>();

    if(statement.step() == SQLITE_ROW)
    {
        create_from_statement(statement, obj);
    }
    else
    {
        logger.error << std::format("Could not create {0} from statement: {1}", name, db->lastErrMsg) << std::endl;
    }

    return obj;
}

template<typename T> void Table<T>::load_entries()
{
    int returnedId = 0;
    Statement statement = select_iter();

    while((returnedId = statement.exec_and_return_id()) > 0)
    {
        std::shared_ptr<T> entry = std::make_shared<T>();

        create_from_statement(statement, entry);

        entries.emplace(returnedId, entry);
    }

    return;
}


// Dummy declarations
template<typename T> int Table<T>::insert(std::shared_ptr<T> entry) { return false; }
template<> int Table<Track>::update(int id, std::shared_ptr<Track> entry) { return false; }

template class Table<Track>;
template class Table<Sequence>;
template class Table<Bank>;
template class Table<Sound>;