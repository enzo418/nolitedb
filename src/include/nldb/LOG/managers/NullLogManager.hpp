#pragma once

#include <cassert>
#include <memory>
#include <stdexcept>

#include "log_constants.hpp"

namespace nldb {
    class LogManager {
       public:
        static void Initialize() {}
        static void Shutdown() {}

        static void SetLevel(log_level::log_level_enum) {}

        static std::shared_ptr<void*> GetLogger() {
            throw std::runtime_error(
                "Logging was disabled, you should not call this function");
        }

       private:
        static std::shared_ptr<void*> logger;

        LogManager() = default;
        ~LogManager() = default;
    };
}  // namespace nldb