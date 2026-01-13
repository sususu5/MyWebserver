#include "protobuf_handler.h"

bool ProtobufHandler::process(Buffer& read_buff, Buffer& write_buff) {
    // TODO: Implement protobuf message processing
    // This is a placeholder implementation
    LOG_DEBUG("ProtobufHandler::process called, bytes available: %zu", read_buff.readable_bytes());
    return false;
}
