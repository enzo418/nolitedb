#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

#include "logger/Logger.h"

typedef std::variant<std::string, int, double> ParamsBindValue;
typedef std::unordered_map<std::string, ParamsBindValue>
    Paramsbind;  // use std::string_view

namespace utils::paramsbind {
    /**
     * @brief Get the Bind Value As String.
     *
     * @param val
     * @param encloseQuotesInString should enclose ' if the value is a string?
     * @return std::string
     */
    std::string getBindValueAsString(const ParamsBindValue& val,
                                     bool encloseQuotesInString = true);

    // this is enough to stop a sql injection
    void encloseQuotes(std::string&);

    // same as above
    std::string encloseQuotesConst(const std::string&);
};  // namespace utils::paramsbind