#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include "protocol.pb.h"

class TcpConnection;

class PushService {
public:
    PushService() = default;
    ~PushService() = default;

    // Session Management
    void add_client(const std::string& user_id, TcpConnection* conn);
    void remove_client(const std::string& user_id);

    // Push Logic
    void push_friend_req(const std::string& sender_id, const std::string& sender_name, const std::string& receiver_id,
                         const std::string& verify_msg);
    void push_friend_status(const std::string& sender_id, const std::string& receiver_id,
                            const std::string& receiver_name, const im::FriendAction& action);
    void push_p2p_message(const im::P2PMessage& msg);

private:
    std::mutex mtx_;
    std::unordered_map<std::string, TcpConnection*> online_connections_;

    void send_envelope(const std::string& receiver_id, const im::Envelope& envelope);
};
