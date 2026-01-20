#include "protobuf_handler.h"
#include <arpa/inet.h>

ProtobufHandler::ProtobufHandler(TcpConnection* conn, AuthService* auth_service, FriendService* friend_service)
    : conn_(conn), auth_service_(auth_service), friend_service_(friend_service) {}

bool ProtobufHandler::process(Buffer& read_buff, Buffer& write_buff) {
    im::Envelope request;

    if (!try_decode_message(read_buff, request)) {
        return false;
    }

    LOG_DEBUG("Received message: cmd={}, seq={}", request.cmd(), request.seq());

    im::Envelope response;
    response.set_seq(request.seq());
    response.set_timestamp(time(nullptr));
    dispatch(request, response);
    encode_message(response, write_buff);

    LOG_DEBUG("Sent response: cmd={}, seq={}", response.cmd(), response.seq());
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
        LOG_ERROR("Message too large: {} bytes (max: {})", msg_len, kMaxMessageSize);
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
        case im::CMD_LOGIN_REQ:
            handle_login(request, response);
            break;
        case im::CMD_ADD_FRIEND_REQ:
            handle_add_friend(request, response);
            break;
        case im::CMD_HANDLE_FRIEND_REQ:
            handle_handle_friend(request, response);
            break;
        case im::CMD_GET_FRIEND_LIST_REQ:
            handle_get_friend_list(request, response);
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
    LOG_INFO("Register request: username={}", req.username());

    im::RegisterResp register_resp;
    auth_service_->user_register(req, &register_resp);
    response.set_cmd(im::CMD_REGISTER_RES);
    response.mutable_register_res()->CopyFrom(register_resp);
}

void ProtobufHandler::handle_login(const im::Envelope& request, im::Envelope& response) {
    if (!request.has_login_req()) {
        LOG_ERROR("CMD_LOGIN_REQ received but payload is missing");
        response.set_cmd(im::CMD_LOGIN_RES);
        auto* resp = response.mutable_login_res();
        resp->set_success(false);
        resp->set_error_msg("Invalid request: missing login payload");
        return;
    }

    const auto& req = request.login_req();
    LOG_INFO("Login request: username={}", req.username());

    im::LoginResp login_resp;
    auth_service_->user_login(conn_, req, &login_resp);
    response.set_cmd(im::CMD_LOGIN_RES);
    response.mutable_login_res()->CopyFrom(login_resp);
}

void ProtobufHandler::handle_add_friend(const im::Envelope& request, im::Envelope& response) {
    if (require_auth(response, im::CMD_ADD_FRIEND_RES)) return;

    if (!request.has_add_friend_req()) {
        LOG_ERROR("CMD_ADD_FRIEND_REQ received but payload is missing");
        response.set_cmd(im::CMD_ADD_FRIEND_RES);
        auto* resp = response.mutable_add_friend_res();
        resp->set_success(false);
        resp->set_error_msg("Invalid request: missing add friend payload");
        return;
    }

    const auto& req = request.add_friend_req();
    LOG_INFO("Add friend request: sender={}, receiver_id={}", current_user_id(), req.receiver_id());

    im::AddFriendResp add_friend_resp;
    friend_service_->add_friend(current_user_id(), req, &add_friend_resp);
    response.set_cmd(im::CMD_ADD_FRIEND_RES);
    response.mutable_add_friend_res()->CopyFrom(add_friend_resp);
}

void ProtobufHandler::handle_handle_friend(const im::Envelope& request, im::Envelope& response) {
    if (require_auth(response, im::CMD_HANDLE_FRIEND_RES)) return;

    if (!request.has_handle_friend_req()) {
        LOG_ERROR("CMD_HANDLE_FRIEND_REQ received but payload is missing");
        response.set_cmd(im::CMD_HANDLE_FRIEND_RES);
        auto* resp = response.mutable_handle_friend_res();
        resp->set_success(false);
        resp->set_error_msg("Invalid request: missing handle friend payload");
        return;
    }

    const auto& req = request.handle_friend_req();
    LOG_INFO("Handle friend request: receiver={}, req_id={}, sender_id={}", current_user_id(), req.req_id(),
             req.sender_id());

    im::HandleFriendResp handle_friend_resp;
    friend_service_->handle_friend(current_user_id(), req, &handle_friend_resp);
    response.set_cmd(im::CMD_HANDLE_FRIEND_RES);
    response.mutable_handle_friend_res()->CopyFrom(handle_friend_resp);
}

void ProtobufHandler::handle_get_friend_list(const im::Envelope& request, im::Envelope& response) {
    if (require_auth(response, im::CMD_GET_FRIEND_LIST_RES)) return;

    if (!request.has_get_friend_list_req()) {
        LOG_ERROR("CMD_GET_FRIEND_LIST_REQ received but payload is missing");
        response.set_cmd(im::CMD_GET_FRIEND_LIST_RES);
        auto* resp = response.mutable_get_friend_list_res();
        resp->set_success(false);
        resp->set_error_msg("Invalid request: missing get friend list payload");
        return;
    }

    LOG_INFO("Get friend list request: user={}", current_user_id());

    im::GetFriendListResp get_friend_list_resp;
    friend_service_->get_friend_list(current_user_id(), &get_friend_list_resp);
    response.set_cmd(im::CMD_GET_FRIEND_LIST_RES);
    response.mutable_get_friend_list_res()->CopyFrom(get_friend_list_resp);
}

void ProtobufHandler::handle_unknown(const im::Envelope& request, im::Envelope& response) {
    LOG_WARN("Unknown command received: {}", request.cmd());
    response.set_cmd(im::CMD_UNKNOWN);
}

bool ProtobufHandler::require_auth(im::Envelope& response, im::CommandType resp_cmd) {
    if (!conn_ || !conn_->is_logged_in()) {
        LOG_WARN("Unauthorized request: user not logged in");
        response.set_cmd(resp_cmd);
        return true;
    }
    return false;
}

const std::string& ProtobufHandler::current_user_id() const {
    static const std::string empty;
    return conn_ ? conn_->get_user_id() : empty;
}
