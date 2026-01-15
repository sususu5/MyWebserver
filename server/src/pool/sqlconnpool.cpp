#include "sqlconnpool.h"

SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool pool;
    return &pool;
}

auto SqlConnPool::Init(const char* host, uint16_t port, const char* user, const char* pwd, const char* dbName,
                       int connSize = 10) -> void {
    assert(connSize > 0);
    auto config = std::make_shared<sqlpp::mysql::connection_config>();
    config->host = host;
    config->port = port;
    config->user = user;
    config->password = pwd;
    config->database = dbName;
    config->ssl = true;
    config->ssl_ca = "/etc/mysql/certs/ca.pem";

    for (int i = 0; i < connSize; i++) {
        try {
            auto conn = new sqlpp::mysql::connection(config);
            connQue_.emplace(conn);
        } catch (const std::exception& e) {
            LOG_ERROR("MYSQL init error: {}", e.what());
        }
    }
    MAX_CONN_ = connSize;
    sem_init(&semId_, 0, MAX_CONN_);

    LOG_INFO("MYSQL connection pool initialized successfully!");
}

auto SqlConnPool::GetConn() -> sqlpp::mysql::connection* {
    sqlpp::mysql::connection* conn = nullptr;
    if (connQue_.empty()) {
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    // Check if the semaphore is greater than 0 which means there are available connections
    // If the semaphore is 0, the thread will be blocked
    sem_wait(&semId_);
    std::lock_guard<std::mutex> locker(mtx_);
    conn = connQue_.front();
    connQue_.pop();
    return conn;
}

auto SqlConnPool::FreeConn(sqlpp::mysql::connection* conn) -> void {
    assert(conn);
    std::lock_guard<std::mutex> locker(mtx_);
    connQue_.push(conn);
    sem_post(&semId_);
}

auto SqlConnPool::ClosePool() -> void {
    std::lock_guard<std::mutex> locker(mtx_);
    while (!connQue_.empty()) {
        auto conn = connQue_.front();
        connQue_.pop();
        delete conn;
    }
}

auto SqlConnPool::GetFreeConnCount() -> int {
    std::lock_guard<std::mutex> locker(mtx_);
    return connQue_.size();
}
