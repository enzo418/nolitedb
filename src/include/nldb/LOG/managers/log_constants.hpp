#pragma once

#define DEFAULT_LOGGER_NAME "nldblog"

namespace nldb::log_level {
    enum log_level_enum : int {
        trace = 0,
        debug,
        info,
        warn,
        err,
        critical,
        off
    };
}