#pragma once

#include <time.h>

#include <array>
#include <bitset>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>  // std::setw
#include <iostream>
#include <mutex>
#include <thread>

#include "nldb/typedef.hpp"

namespace nldb {
    class SnowflakeGenerator {
       public:
        static snowflake generate(uint16_t threadID);

       private:
        static inline snowflake getCurrentTimestampMs() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                .count();
        }
    };
}  // namespace nldb