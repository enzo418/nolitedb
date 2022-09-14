#pragma once
#include <string>
#include <variant>
#include <vector>

#include "CommonTypes.hpp"
#include "Enums.hpp"
#include "ISqlStatement.hpp"
#include "SqlStatement.hpp"

class PropertyRep;

/**
 * @brief Represents what its name says.
 * Holds pointers to the properties that are in the logic expression.
 */
struct SqlLogicExpression : ISqlStatement {
    SqlLogicExpression(std::string st, std::vector<PropertyRep*> pPr);

    std::string getStatement() const override;

    std::vector<PropertyRep*> getActingProps() const;

    SqlLogicExpression operator&&(const SqlLogicExpression& q);

    SqlLogicExpression operator||(const SqlLogicExpression& q);

    SqlLogicExpression operator~();

   protected:
    std::vector<PropertyRep*> actingProps;
    std::string st;
};