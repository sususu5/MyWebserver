#include "protobuf_handler.h"
#include <arpa/inet.h>

ProtobufHandler::ProtobufHandler(AuthService* auth_service) : auth_service_(auth_service) {}

bool ProtobufHandler::process(Buffer& read_buff, Buffer& write_buff) {
    im::Envelope request;

    if (!try_decode_message(read_buff, request)) {
        return false;
    }

    LOG_DEBUG("Received message: cmd=%d, seq=%lu", request.cmd(), request.seq());

    im::Envelope response;
    response.set_seq(request.seq());
    response.set_timestamp(time(nullptr));
    dispatch(request, response);
    encode_message(response, write_buff);

    LOG_DEBUG("Sent response: cmd=%d, seq=%lu", response.cmd(), response.seq());
    return true;
}

bool ProtobufHandler::try_decode_message(Buffer& read_buff, im::Envelope& envelope) {
    if (read_buff.readable_bytes() < kHeaderSize) {
        return false;
    }

    uint32_t msg_len = 0;
    memcpy(&msg_len, read_buff.peek(), kHeaderSize);
    msg_len = ntohl(msg_len);

    if (msg_len > kMaxMessageSize) {
        LOG_ERROR("Message too large: %u bytes (max: %zu)", msg_len, kMaxMessageSize);
        read_buff.retrieve(kHeaderSize);
        return false;
    }

    if (read_buff.readable_bytes() < kHeaderSize + msg_len) {
        return false;
    }

    // Parse the protobuf message
    const char* payload = read_buff.peek() + kHeaderSize;
    if (!envelope.ParseFromArray(payload, msg_len)) {
        LOG_ERROR("Failed to parse protobuf message");
        read_buff.retrieve(kHeaderSize + msg_len);
        return false;
    }

    // Consume the message from buffer
    read_buff.retrieve(kHeaderSize + msg_len);
    return true;
}

void ProtobufHandler::encode_message(const im::Envelope& envelope, Buffer& write_buff) {
    std::string serialized;
    if (!envelope.SerializeToString(&serialized)) {
        LOG_ERROR("Failed to serialize protobuf message");
        return;
    }

    uint32_t msg_len = htonl(static_cast<uint32_t>(serialized.size()));
    write_buff.append(&msg_len, kHeaderSize);
    write_buff.append(serialized.data(), serialized.size());
}

void ProtobufHandler::dispatch(const im::Envelope& request, im::Envelope& response) {
    switch (request.cmd()) {
        case im::CMD_REGISTER_REQ:
            handle_register(request, response);
            break;
        default:
            handle_unknown(request, response);
            break;
    }
}

void ProtobufHandler::handle_register(const im::Envelope& request, im::Envelope& response) {
    if (!request.has_register_req()) {
        LOG_ERROR("CMD_REGISTER_REQ received but payload is missing");
        response.set_cmd(im::CMD_REGISTER_RES);
        auto* resp = response.mutable_register_res();
        resp->set_success(false);
        resp->set_error_msg("Invalid request: missing register payload");
        return;
    }

    const auto& req = request.register_req();
    LOG_INFO("Register request: username=%s", req.username().c_str());

    im::RegisterResp register_resp;
    auth_service_->user_register(req, &register_resp);
    response.set_cmd(im::CMD_REGISTER_RES);
    response.mutable_register_res()->CopyFrom(register_resp);
}

void ProtobufHandler::handle_unknown(const im::Envelope& request, im::Envelope& response) {
    LOG_WARN("Unknown command received: %d", request.cmd());
    response.set_cmd(im::CMD_UNKNOWN);
}
