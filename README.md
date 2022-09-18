# Description
This is an embedded `c++` document oriented data base with all the CRUD operations and with support for most common queries. Its main DTO (data transfer object) is [nlohmann::json](https://github.com/nlohmann/json).

# Features
- Built-in SQLite3 backend support
- Full C++ API, no need to write any SQL string
- CRUD operations
    - Create: insert one or multiple `nlohmann::json`
    - Retrieval: search based on properties keys/values. Returns a `nlohmann::json` array
        - select: select multiple object properties, even aggregate functions like `count`, `max`, `min`, `avg` and `sum`.
        - sort by properties
        - pagination
        - group by properties
    - Update by id; add new properties or update existing
    - Delete by id

# Example
```c++
#include "nlohmann/json.hpp"
#include "dbwrapper/sq3wrapper/DB.hpp"

using namespace nlohmann;

int main() {
    DBSL3 db;

    if (!db.open("./tests.db")) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    // create a query object with the `db` connection and the cars collection
    auto query = QueryFactory::create(&db, "cars");

    json cars = {{{"maker", "ford"}, {"model", "focus"}, {"year", 2011}},
                 {{"maker", "ford"}, {"model", "focus"}, {"year", 2015}},
                 {{"maker", "subaru"}, {"model", "impreza"}, {"year", 2003}}};

    const int totalChanges = query.insert(cars);

    auto [id, model, maker, year] =
        query.prepareProperties("id", "model", "maker", "year");

    // get the first 10 newest model with its id, model name and maker name
    json newestPerModel =
        query.select(id, model, maker, year.maxAs("year_newest_model"))
            .where(year > 1990)
            .page(1, 10)
            .groupBy(model, maker)
            .execute();

    std::cout << "newest per model: " << newestPerModel << std::endl;

    // update first car, set year to 2418 and add a new property called price
    query.update(res1[0]["id"], {{"year", 2418}, {"price", 50000}});

    // remove element with id 1
    int totalChangesDel = query.remove(1);

    // select all cars with all the fields
    auto finalCars = query.select().execute();

    std::cout << "cars after operations: " finalCars << std::endl;    
}
```

## *Note*
You can read more about this project in my [~~devlog~~ 🚧]()

It was intended to serve as a fast development tool for a backend server where I needed an embedded NoSQL database that just worked with json objects out of the box.

# Next
Since this version (1.0) was just to explore how to make it work, it has some problems. For example **working across different databases and collections can fail** due to the way in which we cache the properties. 

So in version 2.0, I expect to solve all the pending problems and actually think about the design. Also, it might work with the SOCI library. All of this without changing the query API, so it's an internal reowork.