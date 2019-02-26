#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

extern std::shared_ptr<spdlog::logger> logger;


#if 1
#define SPDLOG_FILE_LINE_ON
#endif


#ifdef SPDLOG_FILE_LINE_ON
#define SPDLOG_STR_H(x) #x
#define SPDLOG_STR_HELPER(x) SPDLOG_STR_H(x)
#define SPDLOG_INFO(...) logger->info("[" __FILE__ " line #" SPDLOG_STR_HELPER(__LINE__) "] " __VA_ARGS__)
#define SPDLOG_WARN(...) logger->warn("[" __FILE__ " line #" SPDLOG_STR_HELPER(__LINE__) "] " __VA_ARGS__)
#define SPDLOG_ERROR(...) logger->error("[" __FILE__ " line #" SPDLOG_STR_HELPER(__LINE__) "] " __VA_ARGS__)
#else
#define SPDLOG_INFO(...) logger->info(__VA_ARGS__)
#define SPDLOG_WARN(...) logger->warn(__VA_ARGS__)
#define SPDLOG_ERROR(...) logger->error(__VA_ARGS__)
#endif

#ifndef DEBUG
#define DEBUG(...) logger->debug(__VA_ARGS__)
#endif



