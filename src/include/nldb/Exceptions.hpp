#pragma once

#include <stdexcept>

typedef const std::string& str;

class CollectionNotFound : public std::runtime_error {
   public:
    CollectionNotFound(str extra = "")
        : std::runtime_error("Collection not found " + extra) {}
};

class PropertyNotFound : public std::runtime_error {
   public:
    PropertyNotFound(str extra = "")
        : std::runtime_error("Property not found " + extra) {}
};

class DocumentNotFound : public std::runtime_error {
   public:
    DocumentNotFound(str extra = "")
        : std::runtime_error("Document not found " + extra) {}
};

class WrongPropertyType : public std::runtime_error {
   public:
    WrongPropertyType(str pName = "", str pExpected = "", str pActual = "")
        : std::runtime_error("Wrong property type for " + pName +
                             ", expected " + pExpected + " actual " + pActual),
          propertyName(pName),
          expected(pExpected),
          actual(pActual) {}

   public:
    std::string propertyName;
    std::string expected;
    std::string actual;
};