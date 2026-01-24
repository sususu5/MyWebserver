#pragma once

#include <cassandra.h>
#include <cstdint>
#include <mutex>

class ScyllaSession {
public:
    static ScyllaSession* Instance();

    bool Init(const char* host, uint16_t port, const char* user, const char* pwd);
    void Close();

    CassSession* Session();
    bool IsInitialized() const;

private:
    ScyllaSession() = default;
    ~ScyllaSession() = default;

    ScyllaSession(const ScyllaSession&) = delete;
    ScyllaSession& operator=(const ScyllaSession&) = delete;

    mutable std::mutex mtx_;
    CassCluster* cluster_{nullptr};
    CassSession* session_{nullptr};
    bool initialized_{false};
};
