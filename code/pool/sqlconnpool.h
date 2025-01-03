#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

class SqlConnPool {
public:
    static SqlConnPool* Instance();

    MYSQL* GetConn();
    void FreeConn(MYSQL* conn);
    int GetFreeConnCount();

    void Init(const char* host, uint16_t port, const char* user, const char* pwd, const char* dbName, int connSize);
    void ClosePool();

private:
    SqlConnPool() = default;
    ~SqlConnPool() {ClosePool();}

    int MAX_CONN_;

    std::queue<MYSQL*> connQue_;
    std::mutex mtx_;
    sem_t semId_;
};

class SqlConnRAII {
public:
    SqlConnRAII(MYSQL** sql, SqlConnPool* connPool) {
        assert(connPool);
        *sql = connPool->GetConn();
        sql_ = *sql;
        connPool_ = connPool;
    }

    ~SqlConnRAII() {
        if (sql_) {connPool_->FreeConn(sql_);}
    }

private:
    MYSQL* sql_;            // The pointer to the MySQL connection
    SqlConnPool* connPool_; // The pointer to the connection pool
};

#endif