#pragma once
#include <arpa/inet.h>
#include <sys/uio.h>
#include <atomic>
#include <memory>
#include <mutex>
#include "../buffer/buffer.h"

// Forward declarations
class HttpHandler;
class AuthService;
class FriendService;
class PushService;

// ProtocolHandler is a pure virtual class that defines the interface for processing protocol data
class ProtocolHandler {
public:
    virtual ~ProtocolHandler() = default;

    virtual bool process(Buffer& read_buff, Buffer& write_buff) = 0;
    virtual bool is_keep_alive() const { return false; }
};

enum class ConnType {
    HTTP,
    PROTOBUF,
};

class TcpConnection {
public:
    TcpConnection() = default;
    ~TcpConnection();

    void init(int socket_fd, const sockaddr_in& addr);

    ssize_t read(int* error_code);
    ssize_t write(int* error_code);

    void close_conn();

    bool process();

    bool is_keep_alive() const;
    ConnType get_type() const;

    int get_fd() const { return fd_; }
    int get_port() const { return ntohs(addr_.sin_port); }
    // Change machine-readable address to human-readable address
    const char* get_ip() const { return inet_ntoa(addr_.sin_addr); }
    sockaddr_in get_addr() const { return addr_; }

    Buffer& get_read_buffer() { return read_buff_; }
    Buffer& get_write_buffer() { return write_buff_; }

    void send_data(const std::string& data);

    size_t to_write_bytes();

    void set_user_id(const std::string& user_id);
    const std::string& get_user_id() const { return user_id_; }
    bool is_logged_in() const { return !user_id_.empty(); }

    // Represent whether the server is using ET mode
    static bool is_et;
    // Store the source directory of the server
    static const char* src_dir;
    static std::atomic<int> user_count;

    // Service dependencies (injected by Webserver)
    static AuthService* auth_service;
    static FriendService* friend_service;
    static PushService* push_service;

protected:
    int fd_{-1};
    // Store the network address info of the client
    struct sockaddr_in addr_ {};
    bool is_close_{true};

    std::mutex conn_mutex_;
    Buffer read_buff_;
    Buffer write_buff_;
    std::unique_ptr<ProtocolHandler> handler_;

    // iovec for zero-copy file sending (used by HTTP)
    struct iovec iov_[2]{};
    int iov_cnt_{0};

    bool protocol_determined_{false};
    ConnType conn_type_{ConnType::HTTP};

private:
    void setup_iov_for_http();

    std::string user_id_;
};
