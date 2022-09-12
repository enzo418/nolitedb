#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "dbwrapper/IDBQueryReader.hpp"

typedef std::unordered_map<std::string, std::string> Paramsbind;

class IDB {
   public:
    virtual bool open(const std::string& path) = 0;
    virtual bool close() = 0;
    virtual std::unique_ptr<IDBQueryReader> execute(
        const std::string& query, const Paramsbind& params) = 0;

    virtual int getLastInsertedRowId() = 0;

    virtual void throwLastError() = 0;
};