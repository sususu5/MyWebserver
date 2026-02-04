#include "msg_service.h"
#include <ctime>
#include "../dao/async_msg_writer.h"
#include "../log/log.h"

MsgService::MsgService(PushService* push_service) : push_service_(push_service) {
    AsyncMsgWriter::GetInstance()->Start();
}

void MsgService::send_p2p_message(uint64_t sender_id, const im::P2PMessage& req, im::MessageAck* resp) {
    if (sender_id == 0) {
        resp->set_success(false);
        resp->set_error_msg("Sender ID is empty");
        return;
    }
    if (req.receiver_id() == 0) {
        resp->set_success(false);
        resp->set_error_msg("Receiver ID is empty");
        return;
    }
    if (req.timestamp() == 0) {
        resp->set_success(false);
        resp->set_error_msg("Timestamp is empty");
        return;
    }

    AsyncMsgWriter::GetInstance()->Enqueue(req);

    auto msg_to_push = req;
    msg_to_push.set_sender_id(sender_id);

    if (msg_to_push.timestamp() == 0) {
        msg_to_push.set_timestamp(time(nullptr));
    }

    if (push_service_) {
        push_service_->push_p2p_message(msg_to_push);
    }

    resp->set_msg_id(req.msg_id());
    resp->set_success(true);
    resp->set_ref_seq(0);

    LOG_INFO("P2P Message from User[{}] to User[{}] processed.", sender_id, req.receiver_id());
}

void MsgService::sync_messages(uint64_t user_id, const im::SyncMessagesReq& req, im::SyncMessagesResp* resp) {
    if (user_id == 0) {
        resp->set_success(false);
        resp->set_error_msg("User ID is empty");
        return;
    }

    auto messages = msg_scylla_dao_.GetMessagesForUser(user_id);

    resp->set_success(true);
    for (const auto& msg : messages) {
        *resp->add_messages() = msg;
    }

    LOG_INFO("User[{}] synced {} messages (latest 500).", user_id, messages.size());
}
