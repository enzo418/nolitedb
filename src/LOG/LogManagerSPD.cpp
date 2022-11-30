#include "nldb/LOG/managers/LogManagerSPD.hpp"

#include "spdlog/common.h"

namespace nldb {
    std::shared_ptr<spdlog::logger> LogManager::logger;

    void LogManager::Initialize() {
        // console sink
        auto console_sink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        console_sink->set_level(spdlog::level::trace);

        console_sink->set_pattern("%^[%l] %T %D %v%$");

        // file sink for errors
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            "logs/nldb.txt", true);

        file_sink->set_level(spdlog::level::trace);

        // create logger
        std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
        logger = std::make_shared<spdlog::logger>(DEFAULT_LOGGER_NAME,
                                                  sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);

        // register logger
        spdlog::register_logger(logger);
    }

    void LogManager::Shutdown() { spdlog::shutdown(); }

    void LogManager::SetLevel(log_level::log_level_enum level_enum) {
        logger->set_level((spdlog::level::level_enum)level_enum);
    }
}  // namespace nldb