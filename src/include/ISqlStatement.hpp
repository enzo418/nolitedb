#pragma once
#include <string_view>

class ISqlStatement {
   public:
    virtual std::string getStatement() const = 0;
};