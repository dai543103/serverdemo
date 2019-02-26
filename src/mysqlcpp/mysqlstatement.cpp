#include <cassert>

#include "mysqlexception.h"
#include "mysqlstatement.h"

bool mysqlstatement::nextRowset() const
{
    mysql_stmt_free_result(stmt_);
    return mysql_stmt_next_result(stmt_) == 0 && mysql_stmt_field_count(stmt_) > 0;
}

my_ulonglong mysqlstatement::lastInsertId()
{
    return mysql_stmt_insert_id(stmt_);
}

mysqlstatement::mysqlstatement(const char *query, MYSQL *mysql)
    : stmt_(mysql_stmt_init(mysql))//, parameters_(0), fields_(0)
{
    assert(mysql != nullptr);
    if (nullptr == mysql)
        throw mysqlexception("mysql out of memory");

    const size_t length = strlen(query);
    if (0 != mysql_stmt_prepare(stmt_, query, length))
    {
        std::string errormsg(mysqlexception::message(stmt_));
        if (0 != mysql_stmt_free_result(stmt_))
            errormsg += "; There was an error freeing this statement";
        if (0 != mysql_stmt_close(stmt_))
            errormsg += "; There was an error closing this statement";
        throw mysqlexception(errormsg);
    }
}

mysqlstatement::~mysqlstatement()
{
    do 
    {
        mysql_stmt_free_result(stmt_);
    } while (0 == mysql_stmt_next_result(stmt_));

    mysql_stmt_close(stmt_);
}

int mysqlstatement::columnCount() const
{
    return mysql_stmt_field_count(stmt_);
}

int mysqlstatement::paramCount() const
{
    return mysql_stmt_param_count(stmt_);
}
