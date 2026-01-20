#include "webserver.h"
using namespace std;

Webserver::Webserver(int port, int trigMode, int timeoutMS, int sqlPort, const char* sqlUser, const char* sqlPwd,
                     const char* dbName, int connPoolNum, int threadNum, bool openLog, int logLevel, int logQueSize)
    : port_(port), timeoutMS_(timeoutMS), isClose_(false), timer_(new HeapTimer()),
      threadPool_(new ThreadPool(threadNum)), epoller_(new Epoller()), authService_(new AuthService()),
      friendService_(new FriendService()) {
    const char* sql_env_host = getenv("MYSQL_HOST") ? getenv("MYSQL_HOST") : "localhost";

    if (openLog) {
        Log::instance()->init(logLevel, "./log", ".log", logQueSize);
        if (isClose_) {
            LOG_ERROR("========== Server init error!==========");
        } else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Listen Mode: {}, OpenConn Mode: {}", (listenEvent_ & EPOLLET ? "ET" : "LT"),
                     (connEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: {}", logLevel);
            LOG_INFO("srcDir: {}", TcpConnection::src_dir);
            LOG_INFO("MySQL Host: {}", sql_env_host);
            LOG_INFO("SqlConnPool num: {}, ThreadPool num: {}", connPoolNum, threadNum);
        }
    }

    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strcat(srcDir_, "/resources/");
    TcpConnection::user_count = 0;
    TcpConnection::src_dir = srcDir_;
    TcpConnection::auth_service = authService_.get();
    TcpConnection::friend_service = friendService_.get();

    SqlConnPool::Instance()->Init(sql_env_host, sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
    initEventMode_(trigMode);
    if (!initSocket_()) {
        isClose_ = true;
    }
}

Webserver::~Webserver() {
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::Instance()->ClosePool();
}

void Webserver::initEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode) {
        case 0:
            break;
        case 1:
            connEvent_ |= EPOLLET;
            break;
        case 2:
            listenEvent_ |= EPOLLET;
            break;
        case 3:
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
        default:
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
    }
    TcpConnection::is_et = (connEvent_ & EPOLLET);
}

void Webserver::start() {
    int timeMS = -1;
    if (!isClose_) {
        LOG_INFO("========== Server start ==========");
    }
    while (!isClose_) {
        if (timeoutMS_ > 0) {
            timeMS = timer_->getNextTick();
        }
        int eventCnt = epoller_->Wait(timeMS);
        for (int i = 0; i < eventCnt; i++) {
            int fd = epoller_->getEventFd(i);
            uint32_t events = epoller_->getEvents(i);
            // If the file descriptor is the listen socket, deal with the new connection
            if (fd == listenFd_) {
                dealListen_();
            }
            // If the file descriptor is an error, close the connection
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                closeConn_(connections_.at(fd).get());
            }
            // If the file descriptor is readable, deal with the read event
            else if (events & EPOLLIN) {
                assert(connections_.count(fd) > 0);
                dealRead_(connections_[fd].get());
            }
            // If the file descriptor is writable, deal with the write event
            else if (events & EPOLLOUT) {
                assert(connections_.count(fd) > 0);
                dealWrite_(connections_[fd].get());
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void Webserver::sendError_(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {
        LOG_WARN("send error to client[{}] error!", fd);
    }
    close(fd);
}

void Webserver::closeConn_(TcpConnection* client) {
    assert(client);
    LOG_INFO("Client[{}] quit!", client->get_fd());
    epoller_->delFd(client->get_fd());
    client->close_conn();
}

void Webserver::addClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    auto conn = std::make_unique<TcpConnection>();
    conn->init(fd, addr);
    TcpConnection* conn_ptr = conn.get();
    connections_[fd] = std::move(conn);
    if (timeoutMS_ > 0) {
        timer_->add(fd, timeoutMS_, std::bind(&Webserver::closeConn_, this, conn_ptr));
    }
    epoller_->addFd(fd, EPOLLIN | connEvent_);
    setFdNonblock(fd);
    LOG_INFO("Client[{}] in!", conn_ptr->get_fd());
}

void Webserver::dealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    // In ET mode, the loop is used to accept all incoming connections
    do {
        int fd = accept(listenFd_, (struct sockaddr*)&addr, &len);
        if (fd <= 0) {
            return;
        } else if (TcpConnection::user_count >= MAX_FD) {
            sendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        addClient_(fd, addr);
    } while (listenEvent_ & EPOLLET);
}

void Webserver::dealRead_(TcpConnection* client) {
    assert(client);
    extendTime_(client);
    threadPool_->AddTask(std::bind(&Webserver::onRead_, this, client));
}

void Webserver::dealWrite_(TcpConnection* client) {
    assert(client);
    extendTime_(client);
    threadPool_->AddTask(std::bind(&Webserver::onWrite_, this, client));
}

void Webserver::extendTime_(TcpConnection* client) {
    assert(client);
    if (timeoutMS_ > 0) {
        timer_->adjust(client->get_fd(), timeoutMS_);
    }
}

void Webserver::onRead_(TcpConnection* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if (ret <= 0 && readErrno != EAGAIN) {
        closeConn_(client);
        return;
    }
    onProcess_(client);
}

// Resolve the request data
void Webserver::onProcess_(TcpConnection* client) {
    if (client->process()) {
        // If the parsing succeeds, modify the event to EPOLLOUT(write)
        epoller_->modFd(client->get_fd(), connEvent_ | EPOLLOUT);
    } else {
        // If the parsing fails, modify the event to EPOLLIN(read)
        epoller_->modFd(client->get_fd(), connEvent_ | EPOLLIN);
    }
}

void Webserver::onWrite_(TcpConnection* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if (client->to_write_bytes() == 0) {
        // Write completely
        if (client->is_keep_alive()) {
            // If the connection is persistent, modify the event to EPOLLIN
            epoller_->modFd(client->get_fd(), connEvent_ | EPOLLIN);
            return;
        }
    } else if (ret < 0) {
        // The buffer is full
        if (writeErrno == EAGAIN) {
            epoller_->modFd(client->get_fd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    closeConn_(client);
}

bool Webserver::initSocket_() {
    int ret;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    // Create a socket
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        LOG_ERROR("Create socket error! port:{}", port_);
        return false;
    }

    // Set the socket to reuse the address
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (ret == -1) {
        LOG_ERROR("Set socket error!");
        close(listenFd_);
        return false;
    }

    // Bind the socket to the address
    ret = bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_ERROR("Bind Port:{} error!", port_);
        close(listenFd_);
        return false;
    }

    // Start listening
    ret = listen(listenFd_, 8);
    if (ret < 0) {
        LOG_ERROR("Listen port:{} error!", port_);
        close(listenFd_);
        return false;
    }

    // Add the listen socket to the epoll
    ret = epoller_->addFd(listenFd_, EPOLLIN | listenEvent_);
    if (ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    setFdNonblock(listenFd_);
    LOG_INFO("Server port:{}", port_);
    return true;
}

int Webserver::setFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
