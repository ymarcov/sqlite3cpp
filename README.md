### How to use

```c++
Sqlite db{"data.db"};

// query
{
    auto stmt = db.stmt("SELECT Id, Name FROM MyTable WHERE Id=?", 123);

    while (stmt.step()) {
        auto id = stmt.read<int>(0);
        auto name = stmt.read<std::string>(1);

        /* ... */
    }
}

// transaction
{
    auto t = Sqlite::Transaction{db};

    t.transact("INSERT INTO MyTable (Id, Name) VALUES (NULL, ?)", "Yam");
    t.transact("INSERT INTO MyTable (Id, Name) VALUES (NULL, ?)", "Albert");

    t.commit();

    // if any error happened above, everything under 't' will be rolled back.
}
```
