#include "nldb/Utils/ObjectParser.hpp"

#include "nldb/DAL/Repositories.hpp"
#include "nldb/Utils/Variant.hpp"

namespace nldb::utils {

    namespace dotProperties {
        std::vector<Property> readDotSeparatedProperties(
            const std::string& expr, const std::string& collName) {
            std::vector<Property> props;
            unsigned long last_pos = 0;

            while (true) {
                int pos = expr.find('.', last_pos);

                if (pos != (int)expr.npos) {
                    props.push_back(Property {
                        expr.substr(last_pos, pos - last_pos), collName});
                    last_pos = pos + 1;
                } else {
                    if (last_pos < expr.length() - 1) {
                        props.push_back(Property {
                            expr.substr(last_pos, last_pos - expr.length()),
                            collName});
                    }

                    break;
                }
            }

            return props;
        }

    }  // namespace dotProperties

    Object expandObjectExpression(const std::string& str,
                                  const std::string& collName) {
        auto l_it = str.begin();  // create a l value iterator
        // we are sure it's a Object since is the first iteration.
        return std::get<Object>(
            readNextPropertyRecursive(l_it, str.end(), collName));
    }

    // Property readProperty(const std::string& expr,
    //                       const std::string& collName) {
    //     return dotProperties::readDotSeparatedProperties(expr, collName);
    // }
}  // namespace nldb::utils