#include <stdexcept>

#include "nldb/Utils/ParamsBindHelpers.hpp"
#include "nldb/Utils/String.hpp"

namespace utils::paramsbind {
    std::string getBindValueAsString(const ParamsBindValue& val,
                                     bool encloseQuotesInString) {
        if (std::holds_alternative<int>(val)) {
            return std::to_string(std::get<int>(val));
        } else if (std::holds_alternative<double>(val)) {
            return std::to_string(std::get<double>(val));
        } else if (std::holds_alternative<nldb::snowflake>(val)) {
            return std::to_string(std::get<nldb::snowflake>(val));
        } else if (std::holds_alternative<std::string>(val)) {
            if (encloseQuotesInString) {
                return encloseQuotesConst(std::get<std::string>(val));
            } else {
                return std::get<std::string>(val);
            }
        } else if (std::holds_alternative<std::string_view>(val)) {
            if (encloseQuotesInString) {
                return encloseQuotesConst(
                    std::string(std::get<std::string_view>(val)));
            } else {
                return std::string(std::get<std::string_view>(val));
            }
        } else {
            throw std::runtime_error("Type not supported");
        }
    }

    void encloseQuotes(std::string& str) {
        utils::replaceAllOccurrences(str, "\'", "\'\'");
        str = "'" + str + "'";
    }

    std::string encloseQuotesConst(const std::string& s) {
        std::string r = s;
        encloseQuotes(r);
        return r;
    }

    std::string parseSQL(const std::string& str, const Paramsbind& params,
                         bool encloseQuotesInString) {
        std::string sql(str);

        for (auto& param : params) {
            utils::replaceAllOccurrences(
                sql, param.first,
                utils::paramsbind::getBindValueAsString(param.second,
                                                        encloseQuotesInString));
        }

        return sql;
    }
};  // namespace utils::paramsbind