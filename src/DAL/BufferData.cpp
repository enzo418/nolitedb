#include "nldb/DAL/BufferData.hpp"

#include <iostream>

#include "nldb/LOG/log.hpp"
#include "nldb/Profiling/Profiler.hpp"

namespace nldb {

    // how many as you can fit in `bytes`
    template <typename T>
    constexpr int bufferSize(int bytes) {
        return (int)(bytes / sizeof(T));
    }

    template <typename T>
    inline double bufferOccupancy(Buffer<T>& buffer) {
        return (double)buffer.Size() * 100.0 / (double)buffer.Capacity();
    }

    BufferData::BufferData(IDB* pDb, int pSmallBufferSize,
                           int pMediumBufferSize, int pLargeBufferSize)
        : bufferCollection(bufferSize<BufferValueCollection>(pSmallBufferSize)),
          bufferRootProperty(
              bufferSize<BufferValueRootProperty>(pSmallBufferSize)),
          bufferProperty(bufferSize<BufferValueProperty>(pMediumBufferSize)),
          bufferStringLike(bufferSize<BufferValueStringLike>(pLargeBufferSize)),
          bufferDependentObject(
              bufferSize<BufferValueDependentObject>(pMediumBufferSize)),
          bufferIndependentObject(
              bufferSize<BufferValueIndependentObject>(pSmallBufferSize)),
          conn(pDb) {}

    void BufferData::pushPendingData() {
        NLDB_PROFILE_SCOPE("Flush data");

        NLDB_PERF_SUCCESS("FLUSHING PENDING DATA");
        lock.lock();  // next push should wait

        if (bufferRootProperty.Size() > 0) {
            NLDB_PERF_SUCCESS("RootProperty: {}%",
                              bufferOccupancy(bufferRootProperty));
            this->pushRootProperties();
        }

        if (bufferCollection.Size() > 0) {
            NLDB_PERF_SUCCESS("Collection: {}%",
                              bufferOccupancy(bufferCollection));

            this->pushCollections();
        }

        if (bufferProperty.Size() > 0) {
            NLDB_PERF_SUCCESS("Property: {}%", bufferOccupancy(bufferProperty));

            this->pushProperties();
        }

        if (bufferIndependentObject.Size() > 0) {
            NLDB_PERF_SUCCESS("IndependentObject: {}%",
                              bufferOccupancy(bufferIndependentObject));

            this->pushIndependentObjects();
        }

        if (bufferDependentObject.Size() > 0) {
            NLDB_PERF_SUCCESS("DependentObject: {}%",
                              bufferOccupancy(bufferDependentObject));

            this->pushDependentObjects();
        }

        if (bufferStringLike.Size() > 0) {
            NLDB_PERF_SUCCESS("StringLike: {}%",
                              bufferOccupancy(bufferStringLike));

            this->pushStringLikeValues();
        }

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

    BufferData::~BufferData() {}

}  // namespace nldb