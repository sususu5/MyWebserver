#pragma once

#include <semaphore.h>
#include <sqlpp11/mysql/connection.h>
#include <sqlpp11/sqlpp11.h>
#include <mutex>
#include <queue>

class SqlConnPool {
public:
    static SqlConnPool* Instance();

    sqlpp::mysql::connection* GetConn();
    void FreeConn(sqlpp::mysql::connection* conn);
    int GetFreeConnCount();

    void Init(const char* host, uint16_t port, const char* user, const char* pwd, const char* dbName, int connSize);
    void ClosePool();

private:
    SqlConnPool() = default;
    ~SqlConnPool() { ClosePool(); }

    int MAX_CONN_;

    std::queue<sqlpp::mysql::connection*> conn_queue_;
    std::mutex mtx_;
    sem_t semId_;
};

// An RAII wrapper for a MySQL connection
class SqlConnRAII {
public:
    SqlConnRAII(sqlpp::mysql::connection** sql, SqlConnPool* conn_pool) {
        assert(conn_pool);
        *sql = conn_pool->GetConn();
        sql_ = *sql;
        conn_pool_ = conn_pool;
    }

    ~SqlConnRAII() {
        if (sql_ != nullptr) {
            conn_pool_->FreeConn(sql_);
        }
    }

private:
    sqlpp::mysql::connection* sql_;
    SqlConnPool* conn_pool_;
};
