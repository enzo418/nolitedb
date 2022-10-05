#pragma once

#include "nldb/DB/ParameterBinder.hpp"

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

    /**
     * @brief Replaces all the parameters in the string.
     *
     * @param encloseQuotesInString should enclose/replace the quoutes of the
     * parameters values? This is an option since you could have called
     * `encloseQuotes` before.
     *
     * @return std::string
     */
    std::string parseSQL(const std::string&, const Paramsbind&,
                         bool encloseQuotesInString = true);
};  // namespace utils::paramsbind