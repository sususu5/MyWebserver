#pragma once

#include "../dao/friend_dao.h"
#include "../dao/user_dao.h"
#include "friend_service.pb.h"

class PushService;

class FriendService {
public:
    explicit FriendService(PushService* push_service);
    ~FriendService() = default;

    // Add a friend
    void add_friend(const std::string& sender_id, const im::AddFriendReq& req, im::AddFriendResp* resp);
    // Handle a friend request
    void handle_friend(const std::string& receiver_id, const im::HandleFriendReq& req, im::HandleFriendResp* resp);
    // Get a friend list
    void get_friend_list(const std::string& user_id, im::GetFriendListResp* resp);

private:
    FriendDao friend_dao_;
    UserDao user_dao_;
    PushService* push_service_;
};
