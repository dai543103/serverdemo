#pragma once

#include <mysql.h>
#include <vector>
#include <tuple>

#include "mysqlstatement.h"
#include "inputbinder.hpp"
#include "outputbinder.hpp"

class mysqlcpp
{
  public:
    mysqlcpp(const char *const hostname,
             const char *const username,
             const char *const password,
             const char *const database,
             const uint16_t port = 3306);

    ~mysqlcpp();

    mysqlcpp(const mysqlcpp &rhs) = delete;
    mysqlcpp(mysqlcpp &&rhs) = delete;
    mysqlcpp &operator=(const mysqlcpp &rhs) = delete;
    mysqlcpp &operator=(mysqlcpp &&rhs) = delete;

    template <typename... InputArgs, typename... OutputArgs>
    void query(
        std::vector<std::tuple<OutputArgs...>> *const results,
        const char *const query,
        const InputArgs &... args) const;

    template <typename... Args>
    my_ulonglong exec(
        const char *const command,
        const Args &... args);
    my_ulonglong exec(const char *const command);

    statementptr prepare(const char *statement) const;

  private:
    MYSQL *mysql_;
};

template <typename... InputArgs, typename... OutputArgs>
inline void mysqlcpp::query(
    std::vector<std::tuple<OutputArgs...>> *const results,
    const char *const sqlstr,
    const InputArgs &... args) const
{
    assert(nullptr != results);
    assert(nullptr != sqlstr);

    auto statement = prepare(sqlstr);

    statement->execute(std::forward(args)...);

    statement->fetch(results);
}

template <typename... Args>
inline my_ulonglong mysqlcpp::exec(const char *const command, const Args &... args)
{
    auto statement = prepare(command);

    if (0 != statement->columnCount())
        throw mysqlexception("Tried to run query with exec");

    if (sizeof...(args) != statement->paramCount())
    {
        std::string errormsg = "Incorrect number of parameters; command required ";
        errormsg += std::to_string(statement->paramCount());
        errormsg += " but ";
        errormsg += std::to_string(sizeof...(args));
        errormsg += " parameters were provided.";
        throw mysqlexception(errormsg);
    }

    std::vector<MYSQL_BIND> bindparameters(sizeof...(args));
    bindInputs<Args...>(&bindparameters, args...);

    if (0 != mysql_stmt_bind_param(statement->stmt_, bindparameters.data()))
        throw mysqlexception(*statement);

    if (0 != mysql_stmt_execute(statement->stmt_))
        throw mysqlexception(*statement);

    const auto affectedRows = mysql_stmt_affected_rows(statement->stmt_);

    if (static_cast<decltype(affectedRows)>(-1) == affectedRows)
        throw mysqlexception("Tried to run query with exec");

    return affectedRows;
}
