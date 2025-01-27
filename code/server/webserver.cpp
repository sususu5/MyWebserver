#include "webserver.h"
using namespace std;

Webserver::Webserver(
    int port, int trigMode, int timeoutMS, int sqlPort,
    const char* sqlUser, const char* sqlPwd, const char* dbName,
    int connPoolNum, int threadNum, bool openLog, int logLevel,
    int logQueSize):
    port_(port), timeoutMS_(timeoutMS), isClose_(false),
    timer_(new HeapTimer()), threadPool_(new ThreadPool(threadNum)),
    epoller_(new Epoller()) {
    // Whether open the log system
    if (openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if (isClose_) {LOG_ERROR("========== Server init error!==========");}
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                    (listenEvent_ & EPOLLET ? "ET": "LT"),
                    (connEvent_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }

    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strcat(srcDir_, "/resources/");
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;

    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
    initEventMode_(trigMode);
    if (!initSocket_()) {isClose_ = true;}
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
    switch (trigMode){
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
    HttpConn::isET = (connEvent_ & EPOLLET);
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
                closeConn_(&users_.at(fd));
            }
            // If the file descriptor is readable, deal with the read event
            else if (events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                dealRead_(&users_[fd]);
            }
            // If the file descriptor is writable, deal with the write event
            else if (events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                dealWrite_(&users_[fd]);
            }
            else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void Webserver::sendError_(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {LOG_WARN("send error to client[%d] error!", fd);}
    close(fd);
}

void Webserver::closeConn_(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->getFd());
    epoller_->delFd(client->getFd());
    client->Close();
}

void Webserver::addClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if(timeoutMS_ > 0) {
        timer_->add(fd, timeoutMS_, std::bind(&Webserver::closeConn_, this, &users_[fd]));
    }
    epoller_->addFd(fd, EPOLLIN | connEvent_);
    setFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].getFd());
}

void Webserver::dealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    // In ET mode, the loop is used to accept all incoming connections
    do {
        int fd = accept(listenFd_, (struct sockaddr*)&addr, &len);
        if (fd <= 0) {return;}
        else if (HttpConn::userCount >= MAX_FD) {
            sendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        addClient_(fd, addr);
    } while (listenEvent_ & EPOLLET);
}

void Webserver::dealRead_(HttpConn* client) {
    assert(client);
    extendTime_(client);
    threadPool_->AddTask(std::bind(&Webserver::onRead_, this, client));
}

void Webserver::dealWrite_(HttpConn* client) {
    assert(client);
    extendTime_(client);
    threadPool_->AddTask(std::bind(&Webserver::onWrite_, this, client));
}

void Webserver::extendTime_(HttpConn* client) {
    assert(client);
    if (timeoutMS_ > 0) {timer_->adjust(client->getFd(), timeoutMS_);}
}

void Webserver::onRead_(HttpConn* client) {
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

// Resolve the requeest data
void Webserver::onProcess_(HttpConn* client) {
    if (client->process()) {
        epoller_->modFd(client->getFd(), connEvent_ | EPOLLOUT);
    } else {
        epoller_->modFd(client->getFd(), connEvent_ | EPOLLIN);
    }
}

void Webserver::onWrite_(HttpConn* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if (client->toWriteBytes() == 0) {
        // Write completely
        if (client->isKeepAlive()) {
            // If the connection is persistent, modify the event to EPOLLIN
            epoller_->modFd(client->getFd(), connEvent_ | EPOLLIN);
            return;
        }
    } else if (ret < 0) {
        // The buffer is full
        if (writeErrno == EAGAIN) {
            epoller_->modFd(client->getFd(), connEvent_ | EPOLLOUT);
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
        LOG_ERROR("Create socket error!", port_);
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
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    // Start listening
    ret = listen(listenFd_, 8);
    if (ret < 0) {
        LOG_ERROR("Listen port:%d error!", port_);
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
    LOG_INFO("Server port:%d",port_);
    return true;
}

int Webserver::setFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}