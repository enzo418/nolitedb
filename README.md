# Description
This is an embedded `c++` document oriented data base with all the CRUD operations and with support for most common queries. Its main DTO (data transfer object) is [nlohmann::json](https://github.com/nlohmann/json).

# Features
- Multi platforms
- Built-in SQLite3 backend support
- Full C++ API, no need to write any SQL string
- CRUD operations
    - Create: insert one or multiple `nlohmann::json`
    - Retrieval: search based on properties keys/values. Returns a `nlohmann::json` array
        - select: select multiple object properties, even aggregate functions like `count`, `max`, `min`, `avg` and `sum`.
        - sort by properties
        - pagination
        - group by properties
        - join multiple collections
    - Update by id; add new properties or update existing
    - Delete by id

# Examples
check out all the examples in [here](/examples)

```c++
#include "nlohmann/json.hpp"
#include "dbwrapper/sq3wrapper/DB.hpp"

using namespace nlohmann;

int main() {
    DBSL3 db;

    if (!db.open("./cars.db" /*":memory:"*/)) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    // a query object can be used to execute multiple queries.
    Query query = Query(&db);

    // {name, founded, country}
    json data_automaker = {{{"name", "ford"},
                            {"founded", "June 16, 1903"},
                            {"country", "United States"}},
                           // -------------
                           {{"name", "subaru"},
                            {"founded", "July 15, 1953"},
                            {"country", "Japan"}}};

    // {maker, model, year}
    json data_cars = {
        {{"maker", "ford"}, {"model", "focus"}, {"year", 2011}},
        {{"maker", "ford"}, {"model", "focus"}, {"year", 2015}},
        {{"maker", "subaru"}, {"model", "impreza"}, {"year", 2003}}};

    // insert automakers
    query.from("automaker").insert(data_automaker);

    // group all the automaker properties into the `automaker` variable.
    Collection automakers = query.collection("automaker");
    auto automaker = automakers.group("_id", "name", "founded", "country");

    // we can obtain the properties from a collection before inserting any value
    // in it
    Collection cars = query.collection("cars");
    auto [id, model, maker, year] = cars.get("_id", "model", "maker", "year");

    // insert cars data
    query.from("cars").insert(data_cars);

    // select all the cars with manufacturer details
    json all = query.from("cars")
                   .select(id, model, maker, year, automaker)
                   .where(year > 1990 && automaker["name"] == maker)
                   .page(1, 10)
                   .execute();

    std::cout << "\n\nCars with automaker: " << all.dump(2) << std::endl
              << std::endl;

    // Output:
    // [
    //   {
    //       "_id": 3495450966444999936,
    //       "automaker": {
    //          "_id": 3495450965218165120,
    //          "country": "United States",
    //          "founded": "June 16, 1903",
    //          "name": "ford"
    //       },
    //       "maker": "ford",
    //       "model": "focus",
    //       "year": 2011
    //   },
    //   {
    //       "_id": 3495450966447097600,
    //       "automaker": {
    //          "_id": 3495450965218165120,
    //          "country": "United States",
    //          "founded": "June 16, 1903",
    //          "name": "ford"
    //       },
    //       "maker": "ford",
    //       "model": "focus",
    //       "year": 2015
    //   },
    //   {
    //       "_id": 3495450966447097728,
    //       "automaker": {
    //          "_id": 3495450965220262784,
    //          "country": "Japan",
    //          "founded": "July 15, 1953",
    //          "name": "subaru"
    //       },
    //       "maker": "subaru",
    //       "model": "impreza",
    //       "year": 2003
    //   }
    // ]
}
```

# Limits
- Up to 64 properties/members/name value pair per object
    
    This limit comes from the [sqlite maximum number of tables in a join](https://www.sqlite.org/limits.html#:~:text=Maximum%20Number%20Of%20Tables%20In%20A%20Join). When querying an object with `N` members, we make `N` joins to obtain those properties.

- Arrays can only be read and written to

    Because we store them as a string. But nowadays [sqlite3 supports](https://www.sqlite.org/json1.html) some operations over it so maybe we add them in the future.

# Identifiers
The default ids are [snowflake](https://en.wikipedia.org/wiki/Snowflake_ID), since we buffer the values and then insert them all together. This is because they are not yet in the database but anyway we need their ids to relate them.
If you disable the buffer the ids will be the ones assigned by the database, incremental integers.
- snowflake Ids are 64 bits long integers with the
    - first 43 bits to store the timestamp in milliseconds
    - 7 bits for the thread/process id
    - 14 bits thread element counter

## *Note*
You can read more about this project, the process of generating identifiers, performance tests and the inner workings  of it in my [~~devlog~~ 🚧]()
