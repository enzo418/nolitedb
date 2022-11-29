#pragma once

#include <memory>
#include <vector>

#include "log_constants.hpp"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace nldb {

    typedef spdlog::level::level_enum log_level;

    class LogManager {
       public:
        static void Initialize();
        static void Shutdown();

        static void SetLevel(log_level level_enum);

        static std::shared_ptr<spdlog::logger> GetLogger() { return logger; }

       private:
        static std::shared_ptr<spdlog::logger> logger;

        LogManager() = default;
        ~LogManager() = default;
    };
}  // namespace nldb