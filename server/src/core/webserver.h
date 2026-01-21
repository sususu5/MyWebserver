#pragma once

#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include "../pool/threadpool.h"
#include "../service/auth_service.h"
#include "../service/friend_service.h"
#include "../service/push_service.h"
#include "../timer/heaptimer.h"
#include "epoller.h"
#include "tcp_connection.h"

class Webserver {
public:
    Webserver(int port, int trigMode, int timeoutMS, int sqlPort, const char* sqlUser, const char* sqlPwd,
              const char* dbName, int connPoolNum, int threadNum, bool openLog, int logLevel, int logQueSize);
    ~Webserver();
    void start();
    void stop() { isClose_ = true; }

private:
    bool initSocket_();
    void initEventMode_(int trigMode);
    void addClient_(int fd, sockaddr_in addr);

    void dealListen_();
    void dealWrite_(TcpConnection* client);
    void dealRead_(TcpConnection* client);

    void sendError_(int fd, const char* info);
    void extendTime_(TcpConnection* client);
    void closeConn_(TcpConnection* client);

    void onRead_(TcpConnection* client);
    void onWrite_(TcpConnection* client);
    void onProcess_(TcpConnection* client);

    static const int MAX_FD = 65536;
    static int setFdNonblock(int fd);

    int port_;
    bool openLinger_;
    int timeoutMS_;
    bool isClose_;
    int listenFd_;
    char* srcDir_;

    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<Epoller> epoller_;
    std::unique_ptr<PushService> pushService_;
    std::unique_ptr<AuthService> authService_;
    std::unique_ptr<FriendService> friendService_;
    // Store connections by fd, using unique_ptr since TcpConnection is not copyable
    std::unordered_map<int, std::unique_ptr<TcpConnection>> connections_;
};
