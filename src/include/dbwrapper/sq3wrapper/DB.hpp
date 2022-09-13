#include "dbwrapper/IDB.hpp"
#include "dbwrapper/sq3wrapper/DBQueryReader.hpp"
#include "sqlite/sqlite3.h"

class DBSL3 : public IDB {
   public:
    bool open(const std::string& path) override;
    bool close() override;
    std::unique_ptr<IDBQueryReader> executeReader(
        const std::string& query, const Paramsbind& params) override;

    int executeOneStep(const std::string& query,
                       const Paramsbind& params) override;

    std::optional<int> executeAndGetFirstInt(const std::string& query,
                                             const Paramsbind& params) override;

    int executeMultipleOnOneStepRaw(const std::string& query,
                                    const Paramsbind& params) override;

    int getLastInsertedRowId() override;

    std::optional<int> getChangesCount() override;

    void throwLastError() override;

   private:
    sqlite3* db;
};