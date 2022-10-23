#include "nldb/DAL/BufferData.hpp"

#include <iostream>

#include "nldb/LOG/log.hpp"

namespace nldb {

    // how many as you can fit in 0.333 MB
    template <typename T>
    constexpr int bufferSize() {
        return (int)(1e6 / 3 / sizeof(T));
    };

    BufferData::BufferData(IDB* pDb)
        : conn(pDb),
          bufferStringLike(bufferSize<BufferValueStringLike>()),
          bufferDependentObject(bufferSize<BufferValueDependentObject>()),
          bufferIndependentObject(bufferSize<BufferValueIndependentObject>()),
          bufferCollection(bufferSize<BufferValueCollection>()),
          bufferRootProperty(bufferSize<BufferValueRootProperty>()),
          bufferProperty(bufferSize<BufferValueProperty>()) {};

    void BufferData::pushPendingData() {
        NLDB_PERF_SUCCESS("FLUSHING PENDING DATA");
        lock.lock();  // next push should wait

        auto now = std::chrono::high_resolution_clock::now();

        if (bufferRootProperty.Size() > 0) {
            this->pushRootProperties();
        }

        if (bufferCollection.Size() > 0) {
            this->pushCollections();
        }

        if (bufferProperty.Size() > 0) {
            this->pushProperties();
        }

        if (bufferIndependentObject.Size() > 0) {
            this->pushIndependentObjects();
        }

        if (bufferDependentObject.Size() > 0) {
            this->pushDependentObjects();
        }

        if (bufferStringLike.Size() > 0) {
            this->pushStringLikeValues();
        }

        NLDB_PERF_SUCCESS("buffer insert took {} ms",
                          (std::chrono::high_resolution_clock::now() - now) /
                              std::chrono::milliseconds(1),
                          " ms");

        lock.unlock();
    }

    void BufferData::add(const BufferValueCollection& val) {
        _add(val, bufferCollection);
    }

    void BufferData::add(const BufferValueRootProperty& val) {
        _add(val, bufferRootProperty);
    }

    void BufferData::add(const BufferValueProperty& val) {
        _add(val, bufferProperty);
    }

    void BufferData::add(const BufferValueStringLike& val) {
        _add(val, bufferStringLike);
    }

    void BufferData::add(const BufferValueDependentObject& val) {
        _add(val, bufferDependentObject);
    }

    void BufferData::add(const BufferValueIndependentObject& val) {
        _add(val, bufferIndependentObject);
    }

    BufferData::~BufferData() { this->pushPendingData(); }

}  // namespace nldb