#ifndef HTTP_CONN_H;
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include "../log/log.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn {
public:
    HttpConn();
    ~HttpConn();

    void init(int sockFd, const sockaddr_in& addr);
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    void Close();
    int getFd() const;
    int getPort() const;
    const char* getIP() const;
    sockaddr_in getAddr() const;
    bool process();

    int toWriteBytes() {return iov_[0].iov_len + iov_[1].iov_len;}
    bool isKeepAlive() const {return request_.IsKeepAlive();}

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;

private:
    int fd_;
    struct sockaddr_in addr_;
    bool isClose_;
    int iovCnt_;
    struct iovec iov_[2];
    
    Buffer readBuff_;
    Buffer writeBuff_;

    HttpRequest request_;
    HttpResponse response_;
};

#endif