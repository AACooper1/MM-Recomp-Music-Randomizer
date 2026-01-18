#include "table.h"
#include "database.h"

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
Statement Table<T>::select_iter(int id)
{
    
}

template class Table<Track>;