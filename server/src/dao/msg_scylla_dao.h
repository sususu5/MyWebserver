#pragma once

#include "message_service.pb.h"

class MsgScyllaDao {
public:
    MsgScyllaDao() = default;
    ~MsgScyllaDao() = default;

    bool InsertMessage(const im::P2PMessage& msg);
    std::vector<im::P2PMessage> GetMessagesForUser(uint64_t user_id);
};
