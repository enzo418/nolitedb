#pragma once

// use spdlog implementation
#include "managers/LogManagerSPD.hpp"

#ifdef __MINGW32__
#define NLDB_BREAK __debugbreak();
#elif defined(__APPLE__)
#define NLDB_BREAK __builtin_debugtrap();
#elif defined(__linux__)
#define NLDB_BREAK __builtin_trap();
#else

#endif

#ifndef DISABLE_LOGGING
#define NLDB_TRACE(...) nldb::LogManager::GetLogger()->trace(__VA_ARGS__)
#define NLDB_INFO(...) nldb::LogManager::GetLogger()->info(__VA_ARGS__)
#define NLDB_WARN(...) nldb::LogManager::GetLogger()->warn(__VA_ARGS__)
#define NLDB_ERROR(...) nldb::LogManager::GetLogger()->error(__VA_ARGS__)
#define NLDB_CRITICAL(...) nldb::LogManager::GetLogger()->critical(__VA_ARGS__)
#define NLDB_ASSERT(x, msg)                                                 \
    if ((x)) {                                                              \
    } else {                                                                \
        NLDB_CRITICAL("ASSERT FAILED - {}, \n\tFile: {}\n\tLine: {} ", msg, \
                      __FILE__, __LINE__);                                  \
        NLDB_BREAK                                                          \
    }

#if NLDB_DEBUG_CACHE_BUFFER
#define NLDB_PERF_SUCCESS(...) NLDB_INFO(__VA_ARGS__)
#define NLDB_PERF_FAIL(...) NLDB_WARN(__VA_ARGS__)
#else
#define NLDB_PERF_SUCCESS(...) (void)0
#define NLDB_PERF_FAIL(...) (void)0
#endif

#else
#define
#define NLDB_TRACE(...) (void)0
#define NLDB_INFO(...) (void)0
#define NLDB_WARN(...) Observer::LogManager::GetLogger()->warn(__VA_ARGS__)
#define NLDB_ERROR(...) Observer::LogManager::GetLogger()->error(__VA_ARGS__)
#define NLDB_CRITICAL(...) \
    Observer::LogManager::GetLogger()->critical(__VA_ARGS__)
#define NLDB_ASSERT(x, msg) (void)0

#define NLDB_PERF_SUCCESS(...) (void)0
#define NLDB_PERF_FAIL(...) (void)0
#endif