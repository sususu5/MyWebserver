#pragma once

#include <string>
#include "message_service.pb.h"

class MsgScyllaDao {
public:
    MsgScyllaDao() = default;
    ~MsgScyllaDao() = default;

    bool InsertMessage(const std::string& conversation_id, const im::P2PMessage& msg);
};
