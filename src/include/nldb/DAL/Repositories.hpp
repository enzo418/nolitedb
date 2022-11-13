#pragma once

#include <memory>

#include "BufferData.hpp"
#include "IRepositoryCollection.hpp"
#include "IRepositoryProperty.hpp"
#include "IValuesDAO.hpp"

namespace nldb {

    /**
     * @brief Ensures that all the repositories have the same DB connection.
     */
    struct Repositories {
       public:
        void pushPendingData() {
            if (buffered) buffered->pushPendingData();
        }

        std::unique_ptr<IRepositoryCollection> repositoryCollection;
        std::unique_ptr<IRepositoryProperty> repositoryProperty;
        std::unique_ptr<IValuesDAO> valuesDAO;

       protected:
        std::shared_ptr<BufferData> buffered;

       public:
        Repositories(std::unique_ptr<IRepositoryCollection> pRColl,
                     std::unique_ptr<IRepositoryProperty> pRProp,
                     std::unique_ptr<IValuesDAO> pValDAO,
                     std::shared_ptr<BufferData> pBuffered)
            : repositoryCollection(std::move(pRColl)),
              repositoryProperty(std::move(pRProp)),
              valuesDAO(std::move(pValDAO)),
              buffered(std::move(pBuffered)) {}
    };
}  // namespace nldb