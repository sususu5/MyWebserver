#include "webserver.h"
#include "../log/log.h"

Webserver::Webserver(int port, int trig_mode, int timeout_ms, int sql_port, const char* sql_user, const char* sql_pwd,
                     const char* db_name, int conn_pool_num, int thread_num, bool open_log, int log_level,
                     int log_que_size)
    : port_(port), timeout_ms_(timeout_ms), is_close_(false), timer_(new HeapTimer()),
      thread_pool_(new ThreadPool(thread_num)), epoller_(new Epoller()), push_service_(new PushService()),
      auth_service_(new AuthService()), friend_service_(new FriendService(push_service_.get())),
      msg_service_(new MsgService(push_service_.get())) {
    const char* sql_env_host = getenv("MYSQL_HOST") ? getenv("MYSQL_HOST") : "localhost";

    if (open_log) {
        Log::instance()->init(log_level, "./log", ".log", log_que_size);
        if (is_close_) {
            LOG_ERROR("========== Server init error!==========");
        } else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Listen Mode: {}, OpenConn Mode: {}", (listen_event_ & EPOLLET ? "ET" : "LT"),
                     (conn_event_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: {}", log_level);
            LOG_INFO("src_dir: {}", TcpConnection::src_dir);
            LOG_INFO("MySQL Host: {}", sql_env_host);
            LOG_INFO("SqlConnPool num: {}, ThreadPool num: {}", conn_pool_num, thread_num);
        }
    }

    src_dir_ = getcwd(nullptr, 256);
    assert(src_dir_);
    strcat(src_dir_, "/resources/");
    TcpConnection::user_count = 0;
    TcpConnection::src_dir = src_dir_;
    TcpConnection::auth_service = auth_service_.get();
    TcpConnection::friend_service = friend_service_.get();
    TcpConnection::push_service = push_service_.get();
    TcpConnection::msg_service = msg_service_.get();
    TcpConnection::epoller_ = epoller_.get();

    SqlConnPool::Instance()->Init(sql_env_host, sql_port, sql_user, sql_pwd, db_name, conn_pool_num);
    InitEventMode_(trig_mode);
    if (!InitSocket_()) {
        is_close_ = true;
    }
}

Webserver::~Webserver() {
    LOG_INFO("========== Server shutting down ==========");
    close(listen_fd_);
    is_close_ = true;
    free(src_dir_);
    SqlConnPool::Instance()->ClosePool();
    LOG_INFO("========== Server stopped ==========");
    Log::instance()->flush();
}

void Webserver::InitEventMode_(int trig_mode) {
    listen_event_ = EPOLLRDHUP;
    conn_event_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trig_mode) {
        case 0:
            break;
        case 1:
            conn_event_ |= EPOLLET;
            break;
        case 2:
            listen_event_ |= EPOLLET;
            break;
        case 3:
            listen_event_ |= EPOLLET;
            conn_event_ |= EPOLLET;
            break;
        default:
            listen_event_ |= EPOLLET;
            conn_event_ |= EPOLLET;
            break;
    }
    TcpConnection::is_et = (conn_event_ & EPOLLET);
}

void Webserver::Start() {
    int time_ms = -1;
    if (!is_close_) {
        LOG_INFO("========== Server start ==========");
    }
    while (!is_close_) {
        if (timeout_ms_ > 0) {
            time_ms = timer_->getNextTick();
        }
        int event_cnt = epoller_->Wait(time_ms);
        for (int i = 0; i < event_cnt; i++) {
            int fd = epoller_->getEventFd(i);
            uint32_t events = epoller_->getEvents(i);
            // If the file descriptor is the listen socket, deal with the new connection
            if (fd == listen_fd_) {
                DealListen_();
            }
            // If the file descriptor is an error, close the connection
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                CloseConn_(connections_.at(fd).get());
            }
            // If the file descriptor is readable, deal with the read event
            else if (events & EPOLLIN) {
                assert(connections_.count(fd) > 0);
                DealRead_(connections_[fd].get());
            }
            // If the file descriptor is writable, deal with the write event
            else if (events & EPOLLOUT) {
                assert(connections_.count(fd) > 0);
                DealWrite_(connections_[fd].get());
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void Webserver::SendError_(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {
        LOG_WARN("send error to client[{}] error!", fd);
    }
    close(fd);
}

void Webserver::CloseConn_(TcpConnection* client) {
    assert(client);
    LOG_INFO("Client[{}] quit!", client->get_fd());
    epoller_->delFd(client->get_fd());
    client->close_conn();
}

void Webserver::AddClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    auto conn = std::make_unique<TcpConnection>();
    conn->init(fd, addr);
    TcpConnection* conn_ptr = conn.get();
    connections_[fd] = std::move(conn);
    if (timeout_ms_ > 0) {
        timer_->add(fd, timeout_ms_, std::bind(&Webserver::CloseConn_, this, conn_ptr));
    }
    epoller_->addFd(fd, EPOLLIN | conn_event_);
    SetFdNonblock(fd);
    LOG_INFO("Client[{}] in!", conn_ptr->get_fd());
}

void Webserver::DealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    // In ET mode, the loop is used to accept all incoming connections
    do {
        int fd = accept(listen_fd_, (struct sockaddr*)&addr, &len);
        if (fd <= 0) {
            return;
        } else if (TcpConnection::user_count >= MAX_FD) {
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient_(fd, addr);
    } while (listen_event_ & EPOLLET);
}

void Webserver::DealRead_(TcpConnection* client) {
    assert(client);
    ExtendTime_(client);
    thread_pool_->AddTask(std::bind(&Webserver::OnRead_, this, client));
}

void Webserver::DealWrite_(TcpConnection* client) {
    assert(client);
    ExtendTime_(client);
    thread_pool_->AddTask(std::bind(&Webserver::OnWrite_, this, client));
}

void Webserver::ExtendTime_(TcpConnection* client) {
    assert(client);
    if (timeout_ms_ > 0) {
        timer_->adjust(client->get_fd(), timeout_ms_);
    }
}

void Webserver::OnRead_(TcpConnection* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if (ret <= 0 && readErrno != EAGAIN) {
        CloseConn_(client);
        return;
    }
    OnProcess_(client);
}

// Resolve the request data
void Webserver::OnProcess_(TcpConnection* client) {
    if (client->process()) {
        // If the parsing succeeds, modify the event to EPOLLOUT(write)
        epoller_->modFd(client->get_fd(), conn_event_ | EPOLLOUT);
    } else {
        // If the parsing fails, modify the event to EPOLLIN(read)
        epoller_->modFd(client->get_fd(), conn_event_ | EPOLLIN);
    }
}

void Webserver::OnWrite_(TcpConnection* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if (client->to_write_bytes() == 0) {
        // Write completely
        if (client->is_keep_alive()) {
            // If the connection is persistent, modify the event to EPOLLIN
            epoller_->modFd(client->get_fd(), conn_event_ | EPOLLIN);
            return;
        }
    } else if (ret < 0) {
        // The buffer is full
        if (writeErrno == EAGAIN) {
            epoller_->modFd(client->get_fd(), conn_event_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client);
}

bool Webserver::InitSocket_() {
    int ret;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    // Create a socket
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        LOG_ERROR("Create socket error! port:{}", port_);
        return false;
    }

    // Set the socket to reuse the address
    int optval = 1;
    ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (ret == -1) {
        LOG_ERROR("Set socket error!");
        close(listen_fd_);
        return false;
    }

    // Bind the socket to the address
    ret = bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_ERROR("Bind Port:{} error!", port_);
        close(listen_fd_);
        return false;
    }

    // Start listening
    ret = listen(listen_fd_, 8);
    if (ret < 0) {
        LOG_ERROR("Listen port:{} error!", port_);
        close(listen_fd_);
        return false;
    }

    // Add the listen socket to the epoll
    ret = epoller_->addFd(listen_fd_, EPOLLIN | listen_event_);
    if (ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listen_fd_);
        return false;
    }
    SetFdNonblock(listen_fd_);
    LOG_INFO("Server port:{}", port_);
    return true;
}

int Webserver::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
