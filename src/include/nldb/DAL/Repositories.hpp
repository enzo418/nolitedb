#pragma once

#include <memory>

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
    };
}  // namespace nldb