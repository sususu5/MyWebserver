#pragma once

#include <cstdint>
#include "../dao/msg_scylla_dao.h"
#include "message_service.pb.h"
#include "push_service.h"

class MsgService {
public:
    explicit MsgService(PushService* push_service);
    ~MsgService() = default;

    // Send a P2P message
    void send_p2p_message(uint64_t sender_id, const im::P2PMessage& req, im::MessageAck* resp);

private:
    PushService* push_service_;
    MsgScyllaDao msg_scylla_dao_;
};
