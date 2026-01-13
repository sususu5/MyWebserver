#pragma once
#include "../buffer/buffer.h"
#include "tcp_connection.h"

class ProtobufHandler : public ProtocolHandler {
public:
    bool process(Buffer& read_buff, Buffer& write_buff) override;
};
