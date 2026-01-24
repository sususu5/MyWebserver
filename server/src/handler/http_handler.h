#pragma once

#include "../http/httprequest.h"
#include "../http/httpresponse.h"
#include "core/tcp_connection.h"

class HttpHandler : public ProtocolHandler {
public:
    HttpHandler() = default;
    ~HttpHandler() override;

    bool Process(Buffer& read_buff, Buffer& write_buff) override;
    bool IsKeepAlive() const override { return request_.IsKeepAlive(); }

    size_t FileLen() const { return response_.FileLen(); }
    char* File() { return response_.File(); }

private:
    HttpRequest request_;
    HttpResponse response_;
};
