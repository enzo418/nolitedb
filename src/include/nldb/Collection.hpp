#pragma once

#include <string>

namespace nldb {
    class Collection {
       public:
        Collection(int id, const std::string& name);

       public:
        int getId() const;
        std::string getName() const;

       protected:
        int id;
        std::string name;
    };
}  // namespace nldb