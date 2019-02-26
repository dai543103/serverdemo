#pragma once

#include <mysql.h>
#include <vector>
#include <tuple>
#include <memory>
#include <assert.h>

#include "mysqlexception.h"
#include "mysqlstatement.h"

typedef typename std::remove_reference<decltype(*std::declval<
                                                typename std::remove_reference<decltype(
                                                    std::declval<MYSQL_BIND>().length)>::type>())>::type mysql_bind_length_t;

namespace OutputBinderPrivate
{

static const char NULL_VALUE_ERROR_MESSAGE[] =
    "Null value encountered with non-smart-pointer output type";

template <int I>
struct int_
{
};

template <typename Tuple, int I>
void setResultTuple(
    Tuple *const tuple,
    const std::vector<MYSQL_BIND> &bindparameters,
    int_<I>);
template <typename Tuple>
void setResultTuple(
    Tuple *const tuple,
    const std::vector<MYSQL_BIND> &bindparameters,
    int_<-1>);

template <typename T>
struct OutputBinder
{
    static void bind(
        T *const value,
        const MYSQL_BIND &bind);
};

template <typename T>
struct OutputBinder<std::shared_ptr<T>>
{
    static void bind(
        std::shared_ptr<T> *const value,
        const MYSQL_BIND &bind);
};

template <typename T>
struct OutputBinder<std::unique_ptr<T>>
{
    static void bind(
        std::unique_ptr<T> *const value,
        const MYSQL_BIND &bind);
};

template <typename T>
struct OutputBinder<T *>
{
    static void bind(
        T **const value,
        const MYSQL_BIND &bind);
};

template <typename Tuple, int I>
void bindParameters(
    const Tuple &tuple,
    std::vector<MYSQL_BIND> *const bindparameters,
    std::vector<std::vector<char>> *const buffers,
    std::vector<mysql_bind_length_t> *const lengths,
    std::vector<my_bool> *const nullflags,
    int_<I>);

template <typename Tuple>
void bindParameters(
    const Tuple &tuple,
    std::vector<MYSQL_BIND> *const,
    std::vector<std::vector<char>> *const,
    std::vector<mysql_bind_length_t> *const,
    std::vector<my_bool> *const,
    int_<-1>);

template <typename T>
struct OutputBinderParameter
{
    static void bind(
        MYSQL_BIND *const bind,
        std::vector<char> *const buffer,
        mysql_bind_length_t *const length,
        my_bool *const isNullFlag);
};

template <typename T>
struct OutputBinderParameter<std::shared_ptr<T>>
{
    static void bind(
        MYSQL_BIND *const bind,
        std::vector<char> *const buffer,
        mysql_bind_length_t *const length,
        my_bool *const isNullFlag);
};

template <typename T>
struct OutputBinderParameter<std::unique_ptr<T>>
{
    static void bind(
        MYSQL_BIND *const bind,
        std::vector<char> *const buffer,
        mysql_bind_length_t *const length,
        my_bool *const isNullFlag);
};

template <typename T>
struct OutputBinderParameter<T *>
{
    static void bind(
        MYSQL_BIND *const bind,
        std::vector<char> *const buffer,
        mysql_bind_length_t *const length,
        my_bool *const isNullFlag);
};

template <typename Tuple, int I>
void setResultTuple(
    Tuple *const tuple,
    const std::vector<MYSQL_BIND> &bindparameters,
    int_<I>)
{
    OutputBinder<typename std::tuple_element<I, Tuple>::type>::bind(&(std::get<I>(*tuple)), bindparameters.at(I));
    setResultTuple(tuple, bindparameters, int_<I - 1>{});
}

template <typename Tuple>
void setResultTuple(
    Tuple *const,
    const std::vector<MYSQL_BIND> &,
    int_<-1>)
{
}

template <typename Tuple, int I>
void bindParameters(
    const Tuple &tuple,
    std::vector<MYSQL_BIND> *const bindparameters,
    std::vector<std::vector<char>> *const buffers,
    std::vector<mysql_bind_length_t> *const lengths,
    std::vector<my_bool> *const nullflags,
    int_<I>)
{
    OutputBinderParameter<typename std::tuple_element<I, Tuple>::type>::bind(
        &bindparameters->at(I),
        &buffers->at(I),
        &lengths->at(I),
        &nullflags->at(I));
    bindParameters(tuple, bindparameters, buffers, lengths, nullflags, int_<I - 1>());
}

template <typename Tuple>
void bindParameters(
    const Tuple &,
    std::vector<MYSQL_BIND> *const,
    std::vector<std::vector<char>> *const,
    std::vector<mysql_bind_length_t> *const,
    std::vector<my_bool> *const,
    int_<-1>)
{
}

template <typename T>
inline void OutputBinder<T>::bind(T *const value, const MYSQL_BIND &bind)
{
    static_assert(sizeof(T) < 0,
                  "Raw pointers are not supported; use std::shared_ptr or std::unique_ptr instead");
}

template <typename T>
inline void OutputBinder<std::shared_ptr<T>>::bind(std::shared_ptr<T> *const value, const MYSQL_BIND &bind)
{
    if (*bind.is_null)
        value->reset();
    else
    {
        T *newObject = new T();
        OutputBinder<T>::bind(newObject, bind);
        *value = std::shared_ptr<T>(newObject);
    }
}
template <typename T>
inline void OutputBinder<std::unique_ptr<T>>::bind(std::unique_ptr<T> *const value, const MYSQL_BIND &bind)
{
    if (*bind.is_null)
        value->reset();
    else
    {
        T *newObject = new T();
        OutputBinder<T>::bind(newObject, bind);
        *value = std::unique_ptr<T>(newObject);
    }
}
template <typename T>
inline void OutputBinder<T *>::bind(T **const value, const MYSQL_BIND &bind)
{
    static_assert(sizeof(T) < 0,
                  "Raw pointers are not supported; use std::shared_ptr or std::unique_ptr instead");
}

#ifndef OUTPUT_BINDER_SPECIALIZATION
#define OUTPUT_BINDER_SPECIALIZATION(type)                             \
    template <>                                                        \
    struct OutputBinder<type>                                          \
    {                                                                  \
        static void bind(                                              \
            type *const value,                                         \
            const MYSQL_BIND &bind)                                    \
        {                                                              \
            if (*bind.is_null)                                         \
                throw mysqlexception(NULL_VALUE_ERROR_MESSAGE);        \
            *value = *static_cast<const decltype(value)>(bind.buffer); \
        }                                                              \
    };

#endif
OUTPUT_BINDER_SPECIALIZATION(int8_t)
OUTPUT_BINDER_SPECIALIZATION(uint8_t)
OUTPUT_BINDER_SPECIALIZATION(int16_t)
OUTPUT_BINDER_SPECIALIZATION(uint16_t)
OUTPUT_BINDER_SPECIALIZATION(int32_t)
OUTPUT_BINDER_SPECIALIZATION(uint32_t)
OUTPUT_BINDER_SPECIALIZATION(int64_t)
OUTPUT_BINDER_SPECIALIZATION(uint64_t)
OUTPUT_BINDER_SPECIALIZATION(float)
OUTPUT_BINDER_SPECIALIZATION(double)

template <>
struct OutputBinder<std::string>
{
    static void bind(std::string *const value, const MYSQL_BIND &bind)
    {
        if (*bind.is_null)
            throw mysqlexception(NULL_VALUE_ERROR_MESSAGE);
        value->assign(static_cast<const char *>(bind.buffer), *bind.length);
    }
};

template <typename T>
inline void OutputBinderParameter<T>::bind(
    MYSQL_BIND *const bind,
    std::vector<char> *const buffer,
    mysql_bind_length_t *const length,
    my_bool *const isNullFlag)
{
    bind->buffer_type = MYSQL_TYPE_STRING;
    if (0 == buffer->size())
        buffer->resize(20);
    bind->buffer = buffer->data();
    bind->is_null = isNullFlag;
    bind->buffer_length = buffer->size();
    bind->length = length;
}

template <typename T>
inline void OutputBinderParameter<std::shared_ptr<T>>::bind(
    MYSQL_BIND *const bind,
    std::vector<char> *const buffer,
    mysql_bind_length_t *const length,
    my_bool *const isNullFlag)
{
    OutputBinderParameter<T>::bind(bind, buffer, length, isNullFlag);
}

template <typename T>
inline void OutputBinderParameter<std::unique_ptr<T>>::bind(
    MYSQL_BIND *const bind,
    std::vector<char> *const buffer,
    mysql_bind_length_t *const length,
    my_bool *const isNullFlag)
{
    OutputBinderParameter<T>::bind(bind, buffer, length, isNullFlag);
}

template <typename T>
inline void OutputBinderParameter<T *>::bind(
    MYSQL_BIND *const,
    std::vector<char> *const,
    mysql_bind_length_t *const,
    my_bool *const)
{
    static_assert(sizeof(T) < 0,
                  "Raw pointers are not supported; use std::shared_ptr or std::unique_ptr instead");
}

#ifndef OUTPUT_BINDER_PARAMETER_SPECIALIZATION
#define OUTPUT_BINDER_PARAMETER_SPECIALIZATION(TYPE, MYSQLTYPE, ISUNSIGNED) \
    template <>                                                             \
    struct OutputBinderParameter<TYPE>                                      \
    {                                                                       \
        static void bind(                                                   \
            MYSQL_BIND *const bind,                                         \
            std::vector<char> *const buffer,                                \
            mysql_bind_length_t *const length,                              \
            my_bool *const isNullFlag)                                      \
        {                                                                   \
            bind->buffer_type = MYSQLTYPE;                                  \
            buffer->resize(sizeof(TYPE));                                   \
            bind->buffer = buffer->data();                                  \
            bind->is_null = isNullFlag;                                     \
            bind->is_unsigned = ISUNSIGNED;                                 \
            bind->length = length;                                          \
        }                                                                   \
    };

#endif

OUTPUT_BINDER_PARAMETER_SPECIALIZATION(int8_t, MYSQL_TYPE_TINY, 0)
OUTPUT_BINDER_PARAMETER_SPECIALIZATION(uint8_t, MYSQL_TYPE_TINY, 1)
OUTPUT_BINDER_PARAMETER_SPECIALIZATION(int16_t, MYSQL_TYPE_SHORT, 0)
OUTPUT_BINDER_PARAMETER_SPECIALIZATION(uint16_t, MYSQL_TYPE_SHORT, 1)
OUTPUT_BINDER_PARAMETER_SPECIALIZATION(int32_t, MYSQL_TYPE_LONG, 0)
OUTPUT_BINDER_PARAMETER_SPECIALIZATION(uint32_t, MYSQL_TYPE_LONG, 1)
OUTPUT_BINDER_PARAMETER_SPECIALIZATION(int64_t, MYSQL_TYPE_LONGLONG, 0)
OUTPUT_BINDER_PARAMETER_SPECIALIZATION(uint64_t, MYSQL_TYPE_LONGLONG, 1)
OUTPUT_BINDER_PARAMETER_SPECIALIZATION(float, MYSQL_TYPE_FLOAT, 0)
OUTPUT_BINDER_PARAMETER_SPECIALIZATION(double, MYSQL_TYPE_DOUBLE, 0)
}

template<typename ...Args>
inline void bindOutputs(
    MYSQL_STMT* stmt, 
    std::vector<std::tuple<Args...>>* const results)
{
    auto columns = mysql_stmt_field_count(stmt);
    std::vector<MYSQL_BIND> parameters(columns);
    std::vector<std::vector<char>> buffers(columns);
    std::vector<mysql_bind_length_t> lengths(columns);
    std::vector<my_bool> nullFlags(columns);

    for (size_t i = 0; i < parameters.size(); ++i)
    {
        auto bind = &parameters.at(i);
        auto buffer = &buffers.at(i);
        bind->buffer_type = MYSQL_TYPE_STRING;
        if (0 == buffer->size())
            buffer->resize(20);
        bind->buffer = buffer->data();
        bind->is_null = &nullFlags.at(i);
        bind->buffer_length = buffer->size();
        bind->length = &lengths.at(i);
    }

    std::tuple<Args...> unused;
    OutputBinderPrivate::bindParameters(
        unused,
        &parameters,
        &buffers,
        &lengths,
        &nullFlags,
        OutputBinderPrivate::int_<sizeof...(Args)-1>{});

    if (0 != mysql_stmt_bind_result(stmt, parameters.data()))
        throw mysqlexception(mysqlexception::message(stmt));

    int status = mysql_stmt_fetch(stmt);
    while (0 == status || MYSQL_DATA_TRUNCATED == status)
    {
        if (MYSQL_DATA_TRUNCATED == status)
        {
            typedef unsigned int mysql_column_t;
            typedef unsigned long mysql_offset_t;

            std::vector<std::tuple<mysql_column_t, mysql_offset_t>> truncatedColumns;
            for (size_t i = 0; i < lengths.size(); ++i)
            {
                std::vector<char> &buffer = buffers.at(i);
                const size_t untruncateLength = lengths.at(i);
                if (untruncateLength > buffer.size())
                {
                    const size_t alreadyRetrieved = buffer.size();
                    truncatedColumns.push_back(std::tuple<size_t, size_t>(i, alreadyRetrieved));
                    buffer.resize(untruncateLength + 1);
                    MYSQL_BIND &bind = parameters.at(i);
                    bind.buffer = &buffer.at(alreadyRetrieved);
                    bind.buffer_length = buffer.size() - alreadyRetrieved - 1;
                }
            }

            if (truncatedColumns.empty())
                throw mysqlexception("truncated columns empty");

            for (const auto &i : truncatedColumns)
            {
                const auto column = std::get<0>(i);
                const auto offset = std::get<1>(i);
                MYSQL_BIND &parameter = parameters.at(column);
                const int status = mysql_stmt_fetch_column(
                    stmt,
                    &parameter,
                    column,
                    offset);
                if (0 != status)
                    throw mysqlexception(mysqlexception::message(stmt));

                std::vector<char> &buffer = buffers.at(column);
                parameter.buffer = buffer.data();
                parameter.buffer_length = buffer.size();
            }

            if (0 != mysql_stmt_bind_result(stmt, parameters.data()))
            {
                throw mysqlexception(mysqlexception::message(stmt));
            }
        }

        std::tuple<Args...> rowTuple;
        OutputBinderPrivate::setResultTuple(&rowTuple, parameters, OutputBinderPrivate::int_<sizeof...(Args)-1>{});

        results->push_back(std::move(rowTuple));
        status = mysql_stmt_fetch(stmt);
    }

    switch (status)
    {
    case MYSQL_NO_DATA:
        break;
    case 1:
        throw mysqlexception(mysqlexception::message(stmt));
        break;
    default:
        assert(false && "Unknown error code from mysql_stmt_fetch");
        throw mysqlexception(mysqlexception::message(stmt));
    }
}
