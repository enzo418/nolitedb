#pragma once

#include <algorithm>
#include <array>
#include <mutex>
#include <vector>

#include "nldb/Property/Property.hpp"

namespace nldb {
    class NullLock {
       public:
        void lock() {}
        void unlock() {}
        bool try_lock() { return true; }
    };

    template <typename T, class Lock = NullLock>
    class Buffer {
       public:
        /**
         * @brief Construct a new Buffer object
         *
         * @param bufferSize max buffer size
         */
        Buffer(int bufferSize) : bufferSize(bufferSize) {
            elements.resize(bufferSize);
        }

        /**
         * @brief add a new element to the buffer
         *
         * @param data new element
         * @return true if the buffer is not full, the element was added
         * @return bool false if buffer is full, in this case the element
         * couldn't be added.
         */
        bool Add(T data) {
            Guard l(lock);

            // is full
            if (elementCount + 1 > this->bufferSize) return false;

            elements[elementCount] = std::move(data);

            if (elementCount <= bufferSize) {
                elementCount++;
            }

            return true;
        }

        /**
         * @brief Walk through the elements
         *
         * @tparam F
         * @param f e.g.
         * [](const T& t, bool isLast) { std::cout << t.name << std::endl; }
         */
        template <typename F>
        void ForEach(const F& f) {
            Guard l(lock);
            for (int i = 0; i < elementCount; i++) {
                f(elements[i], i == elementCount - 1);
            }
        }

        void Reset() {
            Guard l(lock);

            this->elementCount = 0;
        }

        int Size() {
            Guard l(lock);

            return this->elementCount;
        }

        int Capacity() { return this->bufferSize; }

        using Guard = std::lock_guard<Lock>;

       private:
        std::vector<T> elements;
        int bufferSize;
        int elementCount {0};
        Lock lock;
    };
}  // namespace nldb