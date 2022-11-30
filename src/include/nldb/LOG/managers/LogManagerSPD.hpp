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

    class LogManager {
       public:
        static void Initialize();
        static void Shutdown();

        static void SetLevel(log_level::log_level_enum level_enum);

        static std::shared_ptr<spdlog::logger> GetLogger();

       private:
        static std::shared_ptr<spdlog::logger> logger;

        LogManager() = default;
        ~LogManager() = default;
    };
}  // namespace nldb