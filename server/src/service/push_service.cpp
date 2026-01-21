#include "push_service.h"
#include <ctime>
#include "../core/tcp_connection.h"
#include "../log/log.h"
#include "../util/uuid_generator.h"
#include "message.pb.h"

void PushService::add_client(const std::string& user_id, TcpConnection* conn) {
    std::lock_guard<std::mutex> lock(mtx_);
    online_connections_[user_id] = conn;
    LOG_INFO("User[{}] registered for push service", user_id);
}

void PushService::remove_client(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (online_connections_.contains(user_id)) {
        online_connections_.erase(user_id);
        LOG_INFO("User[{}] unregistered from push service", user_id);
    }
}

void PushService::push_friend_req(const std::string& sender_id, const std::string& sender_name,
                                  const std::string& receiver_id, const std::string& verify_msg) {
    im::Envelope envelope;
    envelope.set_seq(0);
    envelope.set_cmd(im::CMD_FRIEND_REQ_PUSH);
    envelope.set_timestamp(time(nullptr));

    auto* push = envelope.mutable_friend_req_push();
    // TODO: Changed random id into large integer id
    push->set_req_id(UuidGenerator::generate());
    push->set_sender_id(sender_id);
    push->set_sender_name(sender_name);
    push->set_verify_msg(verify_msg);
    push->set_timestamp(time(nullptr));

    send_envelope(receiver_id, envelope);
}

void PushService::send_envelope(const std::string& receiver_id, const im::Envelope& envelope) {
    TcpConnection* conn = nullptr;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (online_connections_.contains(receiver_id)) {
            conn = online_connections_.at(receiver_id);
        }
    }

    if (conn) {
        // TODO: Implement a thread-safe way to send the envelope to the connection (task queue)
        std::string serialized;
        if (envelope.SerializeToString(&serialized)) {
            conn->send_data(serialized);
        }
    } else {
        // TODO: Store offline message?
        LOG_WARN("User[{}] not online, push failed", receiver_id);
    }
}
