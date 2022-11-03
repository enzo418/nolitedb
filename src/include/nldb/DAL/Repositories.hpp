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
        std::unique_ptr<IRepositoryCollection> repositoryCollection;
        std::unique_ptr<IRepositoryProperty> repositoryProperty;
        std::unique_ptr<IValuesDAO> valuesDAO;
        std::shared_ptr<BufferData> buffered;

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