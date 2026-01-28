#pragma once

#include <cstdint>
#include "../dao/friend_dao.h"
#include "../dao/user_dao.h"
#include "friend_service.pb.h"
#include "push_service.h"

class FriendService {
public:
    explicit FriendService(PushService* push_service);
    ~FriendService() = default;

    // Add a friend
    void add_friend(uint64_t sender_id, const im::AddFriendReq& req, im::AddFriendResp* resp);
    // Handle a friend request
    void handle_friend(uint64_t receiver_id, const im::HandleFriendReq& req, im::HandleFriendResp* resp);
    // Get a friend list
    void get_friend_list(uint64_t user_id, im::GetFriendListResp* resp);

private:
    FriendDao friend_dao_;
    UserDao user_dao_;
    PushService* push_service_;
};
