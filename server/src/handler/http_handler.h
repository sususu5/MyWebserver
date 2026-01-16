#pragma once

#include "../http/httprequest.h"
#include "../http/httpresponse.h"
#include "core/tcp_connection.h"

class HttpHandler : public ProtocolHandler {
public:
    HttpHandler() = default;
    ~HttpHandler() override;

    bool process(Buffer& read_buff, Buffer& write_buff) override;
    bool is_keep_alive() const override { return request_.IsKeepAlive(); }

    size_t file_len() const { return response_.FileLen(); }
    char* file() { return response_.File(); }

private:
    HttpRequest request_;
    HttpResponse response_;
};
