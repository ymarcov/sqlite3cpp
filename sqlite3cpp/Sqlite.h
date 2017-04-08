#pragma once

#include <stdexcept>
#include <string>

class sqlite3;
class sqlite3_stmt;

class Sqlite {
public:
    class Statement {
    public:
        Statement();
        Statement(sqlite3_stmt*);
        Statement(Statement&&);
        Statement(const Statement&) = delete;
        Statement& operator=(const Statement&) = delete;
        Statement& operator=(Statement&&);
        virtual ~Statement();

        void reset();
        void clearBindings();

        bool exec();
        bool step();
        template <typename T> void bind(unsigned index, T value);
        template <typename T> T read(unsigned column) const;

        static void bindAll(Statement&, unsigned) {}
        template <typename T, typename... Args>
        static void bindAll(Statement& stmt, unsigned index, T val, Args... args) {
            stmt.bind(index, val);
            bindAll(stmt, ++index, args...);
        }

    protected:
        sqlite3_stmt* stmt;
    };

    class Transaction {
    public:
        enum class Type {
            Deferred,
            Exclusive,
            Immediate
        };

        Transaction(Sqlite&, Type = Type::Deferred);
        ~Transaction();

        void rollback();
        void commit();

    private:
        Sqlite* db;
        bool ended;
    };

    Sqlite(const std::string& file);
    ~Sqlite();

    Statement stmt(const std::string& sql);
    void exec(const std::string& sql);

    template <typename... Args>
    Statement stmt(const std::string& sql, Args... args) {
        Statement s = stmt(sql);
        Statement::bindAll(s, 0, args...);
        return s;
    }

    template <typename... Args>
    void exec(const std::string& sql, Args... args) {
        stmt(sql, args...).exec();
    }

    int lastInsertRowId() const;

private:
    sqlite3_stmt* prepare(const std::string& sql);
    sqlite3* db;
};
