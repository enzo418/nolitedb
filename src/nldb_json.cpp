#include "nldb/nldb_json.hpp"

#include "magic_enum.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    PropertyType JsonTypeToPropertyType(int type) {
        auto t = (json::value_t)type;
        switch (t) {
            case json::value_t::number_unsigned:
            case json::value_t::number_integer:
                return PropertyType::INTEGER;
            case json::value_t::number_float:
                return PropertyType::DOUBLE;
            case json::value_t::string:
                return PropertyType::STRING;
            case json::value_t::object:
                return PropertyType::OBJECT;
            case json::value_t::array:
                return PropertyType::ARRAY;
            default:
                auto msg = "Type is not supported: " +
                           std::string(magic_enum::enum_name((json::value_t)t));
                throw std::runtime_error(msg);
        }
    }

    std::string ValueToString(json& value) {
        PropertyType t = JsonTypeToPropertyType((int)value.type());

        std::string str;
        switch (t) {
            case PropertyType::INTEGER:
                str = std::to_string(value.get<int>());
                break;
            case PropertyType::DOUBLE:
                str = std::to_string(value.get<double>());
                break;
            case PropertyType::STRING:
                str = value.get<std::string>();
                break;
            case PropertyType::ARRAY:
                str = value.dump();
                break;
            case PropertyType::OBJECT:
                throw std::runtime_error(
                    "object type is not allowed to stringify");
                break;
            default:
                throw std::runtime_error("uknown type");
        }

        return str;
    }
}  // namespace nldb