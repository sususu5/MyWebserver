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
    void AddFriend(uint64_t sender_id, const im::AddFriendReq& req, im::AddFriendResp* resp);
    // Handle a friend request
    void HandleFriend(uint64_t receiver_id, const im::HandleFriendReq& req, im::HandleFriendResp* resp);
    // Get a friend list
    void GetFriendList(uint64_t user_id, im::GetFriendListResp* resp);
    // Get pending friend requests
    std::vector<im::FriendReqPush> GetPendingRequests(uint64_t user_id);

private:
    FriendDao friend_dao_;
    UserDao user_dao_;
    PushService* push_service_;
};
