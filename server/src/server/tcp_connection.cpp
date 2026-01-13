#include "tcp_connection.h"
#include <errno.h>
#include <unistd.h>
#include <unordered_set>
#include "http_handler.h"
#include "protobuf_handler.h"

std::atomic<int> TcpConnection::user_count{0};
bool TcpConnection::is_et = false;
const char* TcpConnection::src_dir = "";
AuthService* TcpConnection::auth_service = nullptr;

TcpConnection::~TcpConnection() { close_conn(); }

void TcpConnection::init(int socket_fd, const sockaddr_in& addr) {
    assert(socket_fd > 0);
    user_count++;
    addr_ = addr;
    fd_ = socket_fd;
    write_buff_.retrieve_all();
    read_buff_.retrieve_all();
    is_close_ = false;
    protocol_determined_ = false;
    handler_.reset();
    iov_cnt_ = 0;
    LOG_INFO("Client[%d](%s:%d) in, user_count:%d", fd_, get_ip(), get_port(), (int)user_count);
}

void TcpConnection::close_conn() {
    if (!is_close_) {
        is_close_ = true;
        user_count--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, user_count:%d", fd_, get_ip(), get_port(), (int)user_count);
    }
}

static bool is_http_request(const char* data, size_t len) {
    if (len < 4) {
        return false;
    }
    static const std::unordered_set<std::string> methods = {"GET ", "POST", "HEAD", "PUT ", "DELE"};
    return methods.find(std::string(data, 4)) != methods.end();
}

bool TcpConnection::process() {
    if (!protocol_determined_) {
        if (read_buff_.readable_bytes() == 0) {
            return false;
        }

        const char* data = read_buff_.peek();
        auto len = read_buff_.readable_bytes();
        if (is_http_request(data, len)) {
            handler_ = std::make_unique<HttpHandler>();
            conn_type_ = ConnType::HTTP;
            LOG_INFO("Protocol determined: HTTP");
        } else {
            handler_ = std::make_unique<ProtobufHandler>(auth_service);
            conn_type_ = ConnType::PROTOBUF;
            LOG_INFO("Protocol determined: Protobuf");
        }
        protocol_determined_ = true;
    }

    if (handler_ && handler_->process(read_buff_, write_buff_)) {
        // Setup iov for HTTP file sending
        if (conn_type_ == ConnType::HTTP) {
            setup_iov_for_http();
        }
        return true;
    }
    return false;
}

void TcpConnection::setup_iov_for_http() {
    auto* http_handler = dynamic_cast<HttpHandler*>(handler_.get());
    if (!http_handler) return;

    // iov_[0] for response header in write buffer
    iov_[0].iov_base = const_cast<char*>(write_buff_.peek());
    iov_[0].iov_len = write_buff_.readable_bytes();
    iov_cnt_ = 1;

    // iov_[1] for mmap file content (zero-copy)
    if (http_handler->file_len() > 0 && http_handler->file()) {
        iov_[1].iov_base = http_handler->file();
        iov_[1].iov_len = http_handler->file_len();
        iov_cnt_ = 2;
    }

    LOG_DEBUG("HTTP iov setup: header=%zu, file=%zu", iov_[0].iov_len, iov_cnt_ > 1 ? iov_[1].iov_len : 0);
}

size_t TcpConnection::to_write_bytes() {
    if (conn_type_ == ConnType::HTTP && iov_cnt_ > 0) {
        return iov_[0].iov_len + (iov_cnt_ > 1 ? iov_[1].iov_len : 0);
    }
    return write_buff_.readable_bytes();
}

bool TcpConnection::is_keep_alive() const {
    if (handler_) {
        return handler_->is_keep_alive();
    }
    return false;
}

ConnType TcpConnection::get_type() const { return conn_type_; }

ssize_t TcpConnection::read(int* error_code) {
    ssize_t len = -1;
    do {
        len = read_buff_.read_fd(fd_, error_code);
        if (len <= 0) {
            break;
        }
    } while (is_et);
    return len;
}

ssize_t TcpConnection::write(int* error_code) {
    ssize_t len = -1;

    // Use writev for HTTP to support zero-copy file sending
    if (conn_type_ == ConnType::HTTP && iov_cnt_ > 0) {
        do {
            len = writev(fd_, iov_, iov_cnt_);
            if (len <= 0) {
                *error_code = errno;
                break;
            }
            // All data sent
            if (iov_[0].iov_len + (iov_cnt_ > 1 ? iov_[1].iov_len : 0) == 0) {
                break;
            }
            // Adjust iov after partial write
            if (static_cast<size_t>(len) > iov_[0].iov_len) {
                // Header fully sent, adjust file pointer
                size_t file_sent = len - iov_[0].iov_len;
                iov_[1].iov_base = static_cast<char*>(iov_[1].iov_base) + file_sent;
                iov_[1].iov_len -= file_sent;
                if (iov_[0].iov_len > 0) {
                    write_buff_.retrieve_all();
                    iov_[0].iov_len = 0;
                }
            } else {
                // Header partially sent
                iov_[0].iov_base = static_cast<char*>(iov_[0].iov_base) + len;
                iov_[0].iov_len -= len;
                write_buff_.retrieve(len);
            }
        } while (is_et || to_write_bytes() > 10240);
    } else {
        // Simple buffer write for other protocols
        do {
            len = write_buff_.write_fd(fd_, error_code);
            if (len <= 0) {
                break;
            }
        } while (is_et);
    }
    return len;
}
