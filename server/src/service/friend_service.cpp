#include "friend_service.h"

FriendService::FriendService(PushService* push_service) : push_service_(push_service) {}

void FriendService::AddFriend(uint64_t sender_id, const im::AddFriendReq& req, im::AddFriendResp* resp) {
    auto [result, req_id] = friend_dao_.AddFriend(sender_id, req.receiver_id(), req.verify_msg());

    switch (result) {
        case AddFriendResult::SUCCESS: {
            resp->set_success(true);

            if (push_service_) {
                auto sender = user_dao_.FindById(sender_id);
                push_service_->push_friend_req(req_id, sender_id, sender.username(), req.receiver_id(),
                                               req.verify_msg());
            }
            break;
        }
        case AddFriendResult::ALREADY_EXISTS:
            resp->set_success(false);
            resp->set_error_msg("Friend request already sent or exists");
            break;
        case AddFriendResult::DB_ERROR:
            resp->set_success(false);
            resp->set_error_msg("Internal Database Error");
            break;
    }
}

void FriendService::HandleFriend(uint64_t receiver_id, const im::HandleFriendReq& req, im::HandleFriendResp* resp) {
    bool accept = (req.action() == im::FriendAction::ACTION_ACCEPT);
    auto result = friend_dao_.HandleFriend(receiver_id, req.sender_id(), accept);

    if (result.has_value()) {
        resp->set_success(true);
        resp->set_sender_id(result.value());

        if (push_service_) {
            auto receiver = user_dao_.FindById(receiver_id);
            push_service_->push_friend_status(req.sender_id(), receiver_id, receiver.username(), req.action());
        }
    } else {
        resp->set_success(false);
        resp->set_error_msg("Transaction Failed");
    }
}

void FriendService::GetFriendList(uint64_t user_id, im::GetFriendListResp* resp) {
    bool success = friend_dao_.GetFriendList(user_id, resp);
    if (success) {
        resp->set_success(true);
    } else {
        resp->set_success(false);
        resp->set_error_msg("Internal Database Error");
    }
}

std::vector<im::FriendReqPush> FriendService::GetPendingRequests(uint64_t user_id) {
    return friend_dao_.GetPendingRequests(user_id);
}
