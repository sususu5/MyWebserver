#include "http_handler.h"

HttpHandler::~HttpHandler() { response_.UnmapFile(); }

bool HttpHandler::process(Buffer& read_buff, Buffer& write_buff) {
    request_.Init();

    if (read_buff.readable_bytes() <= 0) {
        return false;
    }

    if (request_.parse(read_buff)) {
        LOG_DEBUG("HTTP request path: %s", request_.path().c_str());
        response_.Init(TcpConnection::src_dir, request_.path(), request_.IsKeepAlive(), 200);
    } else {
        response_.Init(TcpConnection::src_dir, request_.path(), false, 400);
    }

    response_.MakeResponse(write_buff);

    LOG_DEBUG("HTTP response: filesize=%zu, to_write=%zu", response_.FileLen(), write_buff.readable_bytes());
    return true;
}
