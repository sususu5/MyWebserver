#include "msg_service.h"
#include <ctime>
#include "../log/log.h"

MsgService::MsgService(PushService* push_service) : push_service_(push_service) {}

void MsgService::send_p2p_message(const std::string& sender_id, const im::P2PMessage& req, im::MessageAck* resp) {
    if (sender_id.empty()) {
        resp->set_success(false);
        resp->set_error_msg("Sender ID is empty");
        return;
    }
    if (req.receiver_id().empty()) {
        resp->set_success(false);
        resp->set_error_msg("Receiver ID is empty");
        return;
    }
    if (req.timestamp() == 0) {
        resp->set_success(false);
        resp->set_error_msg("Timestamp is empty");
        return;
    }

    auto ok = msg_scylla_dao_.InsertMessage(req.receiver_id(), req);
    if (!ok) {
        resp->set_success(false);
        resp->set_error_msg("Failed to insert message");
        return;
    }

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
