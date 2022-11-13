
#include "nldb/DAL/BufferData.hpp"

namespace nldb {
    constexpr int max_sql3_query_length = 1000000000;

    struct BufferDataSQ3 : public BufferData {
        BufferDataSQ3(IDB* db, int SmallBufferSize, int MediumBufferSize,
                      int LargeBufferSize);

        void pushRootProperties() override;
        void pushCollections() override;
        void pushProperties() override;
        void pushIndependentObjects() override;
        void pushDependentObjects() override;
        void pushStringLikeValues() override;
    };
}  // namespace nldb