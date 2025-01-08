#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "epoller.h"
#include "../timer/heaptimer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../http/httpconn.h"

class Webserver {
public:
    Webserver(
        int port, int trigMode, int timeoutMS, int sqlPort,
        const char* sqlUser, const char* sqlPwd, const char* dbName,
        int connPoolNum, int threadNum, bool openLog, int logLevel,
        int logQueSize
    );
    ~Webserver();
    void start();

private:
    bool initSocket_();
    void initEventMode_(int trigMode);
    void addClient_(int fd, sockaddr_in addr);

    void dealListen_();
    void dealWrite_(HttpConn* client);
    void dealRead_(HttpConn* client);

    void sendError_(int fd, const char* info);
    void extendTime_(HttpConn* client);
    void closeConn_(HttpConn* client);

    void onRead_(HttpConn* client);
    void onWrite_(HttpConn* client);
    void onProcess_(HttpConn* client);

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
    unordered_map<int, HttpConn> users_;
};
    
#endif