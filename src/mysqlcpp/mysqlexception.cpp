#include "mysqlexception.h"
#include "mysqlstatement.h"

mysqlexception::mysqlexception(const std::string &msg)
    : message_(msg)
{
}

mysqlexception::mysqlexception(const MYSQL *const connection)
    : message_(message(connection))
{
}

mysqlexception::mysqlexception(const mysqlstatement &statement)
    : message_(message(statement.stmt_))
{
}

mysqlexception::~mysqlexception() noexcept
{
}

const char * mysqlexception::what() const noexcept
{
    return message_.c_str();
}

const char *mysqlexception::message(const MYSQL *const connection)
{
    const char *const msg = mysql_error(const_cast<MYSQL *>(connection));
    if (msg[0] != '\0')
        return msg;
    return "Unknown error";
}

const char *mysqlexception::message(const MYSQL_STMT *const statement)
{
    const char *const msg = mysql_stmt_error(const_cast<MYSQL_STMT *>(statement));
    if (msg[0] != '\0')
        return msg;
    return "Unknown error";
}
