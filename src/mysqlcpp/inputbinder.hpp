#pragma once

#include <mysql.h>
#include <string>
#include <vector>
#include <memory>
#include <string.h>

template <typename... Args>
void bindInputs(
    std::vector<MYSQL_BIND> *inputBindParameters,
    const Args &... args);

namespace InputBinderPrivate
{
template <size_t N, typename... Args>
struct InputBinder
{
    static void bind(std::vector<MYSQL_BIND> *const) {}
};

template <size_t N, typename Head, typename... Tail>
struct InputBinder<N, Head, Tail...>
{
    static void bind(
        std::vector<MYSQL_BIND> *const,
        const Head &,
        const Tail &...)
    {
        static_assert(sizeof...(Tail) < 0,
                      "All types need to have partial template specialized instances defined for them, but one is missing for type Head.");
    }
};

// ************************************************
// Partial template specialization for char pointer
// ************************************************
template <size_t N, typename... Tail>
struct InputBinder<N, char *, Tail...>
{
    static void bind(
        std::vector<MYSQL_BIND> *const bindparameters,
        const char *const &value,
        const Tail &... tail)
    {
        MYSQL_BIND &bindparameter = bindparameters->at(N);

        bindparameter.buffer_type = MYSQL_TYPE_STRING;
        bindparameter.buffer = const_cast<void *>(static_cast<const void *>(value));
        bindparameter.buffer_length = strlen(value);
        bindparameter.length = &bindparameter.buffer_length;
        bindparameter.is_unsigned = 0;
        bindparameter.is_null = 0;

        InputBinder<N + 1, Tail...>::bind(bindparameters, tail...);
    }
};

template <size_t N, typename... Tail>
struct InputBinder<N, const char *, Tail...>
{
    static void bind(
        std::vector<MYSQL_BIND> *const bindparameters,
        const char *const &value,
        const Tail &... tail)
    {
        InputBinder<N, char *, Tail...>::bind(bindparameters, const_cast<char *>(value), tail...);
    }
};

template <size_t N, size_t M, typename... Tail>
struct InputBinder<N, char[M], Tail...>
{
    static void bind(
        std::vector<MYSQL_BIND> *const bindparameters,
        const char(&value)[M],
        const Tail &... tail)
    {
        InputBinder<N, char *, Tail...>::bind(bindparameters, const_cast<char *>(&value[0]), tail...);
    }
};

// ******************************************
// Partial template specialization for string
// ******************************************
template <size_t N, typename... Tail>
struct InputBinder<N, std::string, Tail...>
{
    static void bind(
        std::vector<MYSQL_BIND> *const bindparameters,
        const std::string &value,
        const Tail &... tail)
    {
        MYSQL_BIND &bindparameter = bindparameters->at(N);

        bindparameter.buffer_type = MYSQL_TYPE_STRING;
        bindparameter.buffer = const_cast<void *>(static_cast<const void *>(value.data()));
        bindparameter.buffer_length = value.length();
        bindparameter.length = &bindparameter.buffer_length;
        bindparameter.is_unsigned = 0;
        bindparameter.is_null = 0;

        InputBinder<N + 1, Tail...>::bind(bindparameters, tail...);
    }
};

template <size_t N, typename... Tail>
struct InputBinder<N, std::shared_ptr<std::string>, Tail...>
{
    static void bind(
        std::vector<MYSQL_BIND> *const bindparameters,
        const std::shared_ptr<std::string> &value,
        const Tail &... tail)
    {
        if (value)
            InputBinder<N, std::string, Tail...>::bind(bindparameters, *value, tail...);
        else
        {
            MYSQL_BIND &bindparameter = bindparameters->at(N);
            bindparameter.buffer_type = MYSQL_TYPE_NULL;
            InputBinder<N + 1, Tail...>::bind(bindparameters, tail...);
        }
    }
};

template <size_t N, typename... Tail>
struct InputBinder<N, std::unique_ptr<std::string>, Tail...>
{
    static void bind(
        std::vector<MYSQL_BIND> *const bindparameters,
        const std::unique_ptr<std::string> &value,
        const Tail &... tail)
    {
        if (value)
            InputBinder<N, std::string, Tail...>::bind(bindparameters, *value, tail...);
        else
        {
            MYSQL_BIND &bindparameter = bindparameters->at(N);
            bindparameter.buffer_type = MYSQL_TYPE_NULL;
            InputBinder<N + 1, Tail...>::bind(bindparameters, tail...);
        }
    }
};

template <size_t N, typename... Tail>
struct InputBinder<N, std::nullptr_t, Tail...>
{
    static void bind(
        std::vector<MYSQL_BIND> *const bindparameters,
        const std::nullptr_t &value,
        const Tail &... tail)
    {
        MYSQL_BIND &bindparameter = bindparameters->at(N);
        bindparameter.buffer_type = MYSQL_TYPE_NULL;
        InputBinder<N + 1, Tail...>::bind(bindparameters, tail...);
    }
};

#ifndef INPUT_BINDER_INTEGRAL_TYPE_SPECIALIZATION
#define INPUT_BINDER_INTEGRAL_TYPE_SPECIALIZATION(TYPE, MYSQLTYPE, ISUNSIGNED)            \
    template <size_t N, typename... Tail>                                                 \
    struct InputBinder<N, TYPE, Tail...>                                                  \
    {                                                                                     \
        static void bind(                                                                 \
            std::vector<MYSQL_BIND> *const bindparameters,                                \
            const TYPE &value,                                                            \
            const Tail &... tail)                                                         \
        {                                                                                 \
            MYSQL_BIND &bindparameter = bindparameters->at(N);                            \
            bindparameter.buffer_type = MYSQLTYPE;                                        \
            bindparameter.buffer = const_cast<void *>(static_cast<const void *>(&value)); \
            bindparameter.is_unsigned = ISUNSIGNED;                                       \
            bindparameter.is_null = 0;                                                    \
            InputBinder<N + 1, Tail...>::bind(bindparameters, tail...);                   \
        }                                                                                 \
    };
#endif
INPUT_BINDER_INTEGRAL_TYPE_SPECIALIZATION(int8_t, MYSQL_TYPE_TINY, 0)
INPUT_BINDER_INTEGRAL_TYPE_SPECIALIZATION(uint8_t, MYSQL_TYPE_TINY, 1)
INPUT_BINDER_INTEGRAL_TYPE_SPECIALIZATION(int16_t, MYSQL_TYPE_SHORT, 0)
INPUT_BINDER_INTEGRAL_TYPE_SPECIALIZATION(uint16_t, MYSQL_TYPE_SHORT, 1)
INPUT_BINDER_INTEGRAL_TYPE_SPECIALIZATION(int32_t, MYSQL_TYPE_LONG, 0)
INPUT_BINDER_INTEGRAL_TYPE_SPECIALIZATION(uint32_t, MYSQL_TYPE_LONG, 1)
INPUT_BINDER_INTEGRAL_TYPE_SPECIALIZATION(int64_t, MYSQL_TYPE_LONGLONG, 0)
INPUT_BINDER_INTEGRAL_TYPE_SPECIALIZATION(uint64_t, MYSQL_TYPE_LONGLONG, 1)

#ifndef INPUT_BINDER_FLOATING_TYPE_SPECIALIZATION
#define INPUT_BINDER_FLOATING_TYPE_SPECIALIZATION(TYPE, MYSQLTYPE, SIZE)                  \
    template <size_t N, typename... Tail>                                                 \
    struct InputBinder<N, TYPE, Tail...>                                                  \
    {                                                                                     \
        static void bind(                                                                 \
            std::vector<MYSQL_BIND> *const bindparameters,                                \
            const TYPE &value,                                                            \
            const Tail &... tail)                                                         \
        {                                                                                 \
            static_assert(SIZE == sizeof(TYPE), "Unexpected floating point size");        \
            MYSQL_BIND &bindparameter = bindparameters->at(N);                            \
            bindparameter.buffer_type = MYSQLTYPE;                                        \
            bindparameter.buffer = const_cast<void *>(static_cast<const void *>(&value)); \
            bindparameter.is_null = 0;                                                    \
            InputBinder<N + 1, Tail...>::bind(bindparameters, tail...);                   \
        }                                                                                 \
    };
#endif
INPUT_BINDER_FLOATING_TYPE_SPECIALIZATION(float, MYSQL_TYPE_FLOAT, 4)
INPUT_BINDER_FLOATING_TYPE_SPECIALIZATION(double, MYSQL_TYPE_DOUBLE, 8)
}

template <typename... Args>
inline void bindInputs(std::vector<MYSQL_BIND> *inputBindParameters, const Args &... args)
{
    InputBinderPrivate::InputBinder<0, Args...>::bind(
        inputBindParameters, args...);
}
