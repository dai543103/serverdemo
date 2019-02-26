#pragma once

#include <mysql.h>
#include <string>
#include <exception>

class mysqlstatement;
class mysqlexception : public std::exception
{
  public:
    explicit mysqlexception(const std::string &msg);
    explicit mysqlexception(const MYSQL *const connection);
    explicit mysqlexception(const mysqlstatement &statement);
    ~mysqlexception() noexcept;

    const char *what() const noexcept;

    static const char *message(const MYSQL *const connection);
    static const char *message(const MYSQL_STMT *const statement);

  private:
    const std::string message_;
};
