#include "Sqlite.h"

#include <cstring>
#include <sqlite3.h>
#include <stdexcept>

/*
 * DB::Statement
 */

Sqlite::Statement::Statement()
    : stmt{nullptr} {}

Sqlite::Statement::Statement(sqlite3_stmt* stmt)
    : stmt{stmt} {
    if (!stmt)
        throw std::runtime_error("Failed to prepare SQLite statement");
}

Sqlite::Statement::Statement(Sqlite::Statement&& other)
    : stmt{nullptr} {
    std::swap(stmt, other.stmt);
}

Sqlite::Statement& Sqlite::Statement::operator=(Sqlite::Statement&& other) {
    ::sqlite3_finalize(stmt);
    stmt = other.stmt;
    other.stmt = nullptr;
    return *this;
}

Sqlite::Statement::~Statement() {
    ::sqlite3_finalize(stmt);
}

void Sqlite::Statement::reset() {
    if (::sqlite3_reset(stmt) != SQLITE_OK)
        throw std::runtime_error("Failed to reset statement");
}

void Sqlite::Statement::clearBindings() {
    if (::sqlite3_clear_bindings(stmt) != SQLITE_OK)
        throw std::runtime_error("Failed to clear statment bindings");
}

bool Sqlite::Statement::step() {
    return ::sqlite3_step(stmt) == SQLITE_ROW;
}

bool Sqlite::Statement::exec() {
    return ::sqlite3_step(stmt) == SQLITE_DONE;
}

/**
 *
 * Read specializations
 *
 */

template <> int Sqlite::Statement::read(unsigned column) const {
    return ::sqlite3_column_int(stmt, (int)column);
}

template <> std::string Sqlite::Statement::read(unsigned column) const {
    return (const char*)::sqlite3_column_text(stmt, (int)column);
}

/**
 *
 * Bind specializations
 *
 */

template <> void Sqlite::Statement::bind(unsigned index, const char* value) {
    ::sqlite3_bind_text(stmt, (int)++index, value, std::strlen(value), SQLITE_TRANSIENT);
}

template <> void Sqlite::Statement::bind(unsigned index, std::string value) {
    ::sqlite3_bind_text(stmt, (int)++index, value.c_str(), value.length(), SQLITE_TRANSIENT);
}

template <> void Sqlite::Statement::bind(unsigned index, int value) {
    ::sqlite3_bind_int(stmt, (int)++index, value);
}

template <> void Sqlite::Statement::bind(unsigned index, std::int64_t value) {
    ::sqlite3_bind_int64(stmt, (int)++index, value);
}

/*
 * DB::Transaction
 */

Sqlite::Transaction::Transaction(Sqlite& db, Sqlite::Transaction::Type type) :
    db{&db},
    ended{false} {
    static const char* deferredSql = "BEGIN DEFERRED TRANSACTION";
    static const char* exclusiveSql = "BEGIN EXCLUSIVE TRANSACTION";
    static const char* immediateSql = "BEGIN IMMEDIATE TRANSACTION";

    std::string sql;

    switch (type) {
    case Sqlite::Transaction::Type::Deferred:
        sql = deferredSql; break;
    case Sqlite::Transaction::Type::Exclusive:
        sql = exclusiveSql; break;
    case Sqlite::Transaction::Type::Immediate:
        sql = immediateSql; break;
    }

    db.exec(sql);
}

Sqlite::Transaction::~Transaction() {
    if (!ended)
        rollback();
}

void Sqlite::Transaction::rollback() {
    if (ended)
        throw std::logic_error("Transaction already ended");

    db->exec("ROLLBACK");
    ended = true;
}

void Sqlite::Transaction::commit() {
    if (ended)
        throw std::logic_error("Transaction already ended");

    db->exec("COMMIT");
    ended = true;
}

/*
 * DB
 */

Sqlite::Sqlite(const std::string& file) {
    if (::sqlite3_open(file.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error("Failed to open SQLite DB");
}

Sqlite::~Sqlite() {
    ::sqlite3_close_v2(db);
}

Sqlite::Statement Sqlite::stmt(const std::string& sql) {
    return Sqlite::Statement(prepare(sql));
}

void Sqlite::exec(const std::string& sql) {
    bool result = false;

    if (::sqlite3_stmt* stmt = prepare(sql)) {
        result = ::sqlite3_step(stmt) == SQLITE_DONE;
        ::sqlite3_finalize(stmt);
    }

    if (!result)
        throw std::runtime_error("Failed to execute SQL");
}

int Sqlite::lastInsertRowId() const {
    return ::sqlite3_last_insert_rowid(db);
}

sqlite3_stmt* Sqlite::prepare(const std::string& sql) {
    ::sqlite3_stmt* stmt = nullptr;
    ::sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, nullptr);
    return stmt;
}
