#pragma once

#include <mysql.h>
#include <vector>

#include "inputbinder.hpp"
#include "outputbinder.hpp"

class mysqlstatement
{
  public:
    mysqlstatement(mysqlstatement &&rhs) = default;
    ~mysqlstatement();

    int columnCount() const;

    int paramCount() const;

    template <typename... InputArgs>
    void execute(const InputArgs &...) const;

    template <typename... OutputArgs>
    void fetch(std::vector<std::tuple<OutputArgs...>> *const results) const;

    bool nextRowset() const;

    my_ulonglong lastInsertId();

  private:
    friend class mysqlcpp;
    friend class mysqlexception;

    mysqlstatement(const char *query, MYSQL *mysql);

    mysqlstatement() = delete;
    mysqlstatement(const mysqlstatement &) = delete;
    mysqlstatement &operator=(const mysqlstatement &) = delete;
    mysqlstatement &operator=(mysqlstatement &&) = delete;
    MYSQL_STMT *stmt_;
};

using statementptr = std::shared_ptr<mysqlstatement>;

template<typename ...InputArgs>
inline void mysqlstatement::execute(const InputArgs & ... args) const
{
    if (sizeof...(InputArgs) != mysql_stmt_param_count(stmt_))
    {
        std::string errormsg = "Incorrect number of parameters; command required ";
        errormsg += std::to_string(mysql_stmt_param_count(stmt_));
        errormsg += " but ";
        errormsg += std::to_string(sizeof...(InputArgs));
        errormsg += " parameters were provided.";
        throw mysqlexception(errormsg);
    }

    do
    {
        mysql_stmt_free_result(stmt_);
    } while (0 == mysql_stmt_next_result(stmt_));

    std::vector<MYSQL_BIND> bindparameters(sizeof...(InputArgs));
    bindInputs<InputArgs...>(&bindparameters, args...);

    if (0 != mysql_stmt_bind_param(stmt_, bindparameters.data()))
        throw mysqlexception(mysqlexception::message(stmt_));

    if (0 != mysql_stmt_execute(stmt_))
        throw mysqlexception(mysqlexception::message(stmt_));
}

template<typename ...OutputArgs>
inline void mysqlstatement::fetch(std::vector<std::tuple<OutputArgs...>>* const results) const
{
    //if (sizeof...(OutputArgs) != mysql_stmt_field_count(stmt_))
    //{
    //    std::string errmsg("Incorrect number of output parameters; query required ");
    //    errmsg += std::to_string(mysql_stmt_field_count(stmt_));
    //    errmsg += " but " + std::to_string(sizeof...(OutputArgs));
    //    errmsg += " parameters were provided";
    //    throw mysqlexception(errmsg);
    //}

    bindOutputs<OutputArgs...>(stmt_, results);
}

