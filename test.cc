#include "Sqlite.h"

#include <cassert>
#include <stdexcept>
#include <unistd.h>

int main() {
    // delete initial file, if it exists
    ::unlink("data.db");

    auto db = Sqlite{"data.db"};

    db.exec("CREATE TABLE IF NOT EXISTS MyTable (Id INTEGER PRIMARY KEY, Name TEXT);");

    // do a transaction with an error in the middle
    try {
        auto t = Sqlite::Transaction{db};
        db.exec("INSERT INTO MyTable (Id, Name) VALUES (NULL, ?)", "Yam");
        throw std::runtime_error("Oops");
        // transaction should be rolled back
    } catch (...) {}

    // do a transaction, but don't call commit()
    try {
        auto t = Sqlite::Transaction{db};
        db.exec("INSERT INTO MyTable (Id, Name) VALUES (NULL, ?)", "Yam");
    } catch (...) {}

    // since transaction was rolled back, we should see count == 0
    {
        auto stmt = db.stmt("SELECT COUNT() FROM MyTable");
        assert(stmt.step());
        assert(stmt.read<int>(0) == 0);
    }

    // now let's really insert a couple of rows
    {
        auto t = Sqlite::Transaction{db};

        auto stmt = db.stmt("INSERT INTO MyTable (Id, Name) VALUES (NULL, ?)");

        stmt.bind(0, "Yam");
        stmt.exec();

        stmt.reset();

        stmt.bind(0, "Albert");
        stmt.exec();

        t.commit();
    }

    {
        auto stmt = db.stmt("SELECT COUNT() FROM MyTable");
        assert(stmt.step());
        assert(stmt.read<int>(0) == 2);
    }
}
