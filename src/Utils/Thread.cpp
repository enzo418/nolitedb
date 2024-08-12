#include "nldb/Utils/Thread.hpp"

#include <mutex>
#include <thread>

namespace nldb {
    std::atomic<uint16_t> threadCounter {0};
    thread_local uint16_t threadID = 0;

    // It makes sense to do it this way because of MAX_THREADS = 127
    // ~42µs
    uint16_t getThreadID() {
        if (threadID == 0) {
            threadID = ++threadCounter;
        }
        return threadID % 127;
    }
}  // namespace nldb