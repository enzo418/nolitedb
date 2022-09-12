#include "dbwrapper/IDB.hpp"
#include "dbwrapper/sq3wrapper/DBQueryReader.hpp"
#include "sqlite/sqlite3.h"

class DBSL3 : public IDB {
   public:
    bool open(const std::string& path) override;
    bool close() override;
    std::unique_ptr<IDBQueryReader> execute(const std::string& query,
                                            const Paramsbind& params) override;

    int getLastInsertedRowId() override;

    void throwLastError() override;

   private:
    sqlite3* db;
};