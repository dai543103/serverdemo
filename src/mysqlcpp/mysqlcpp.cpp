#include "mysqlcpp.h"

mysqlcpp::mysqlcpp(const char *const hostname, const char *const username, const char *const password, const char *const database, const uint16_t port)
    : mysql_(mysql_init(nullptr))
{
    if (nullptr == mysql_)
        throw mysqlexception("Unable to connect to mysql");

    const MYSQL *const success = mysql_real_connect(
        mysql_,
        hostname,
        username,
        password,
        database,
        port,
        nullptr,
        CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS | CLIENT_FOUND_ROWS);
    if (nullptr == success)
    {
        mysqlexception ex(mysql_);
        mysql_close(mysql_);
        throw ex;
    }
}

mysqlcpp::~mysqlcpp()
{
    mysql_close(mysql_);
}

my_ulonglong mysqlcpp::exec(const char *const command)
{
    if (0 != mysql_real_query(mysql_, command, strlen(command)))
        throw mysqlexception(mysql_);

    auto affectedRows = mysql_affected_rows(mysql_);
    if ((my_ulonglong)-1 == affectedRows)
    {
        mysql_free_result(mysql_store_result(mysql_));
        throw mysqlexception("Tried to run query with runCommand");
    }
    return affectedRows;
}

statementptr mysqlcpp::prepare(const char *statement) const
{
    return std::shared_ptr<mysqlstatement>(new mysqlstatement(statement, mysql_));
}
