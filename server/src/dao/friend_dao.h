#pragma once

#include <cstdint>
#include <optional>
#include "base_dao.h"
#include "friend_service.pb.h"
#include "model/schema.h"

// Result codes for add_friend operation
enum class AddFriendResult { SUCCESS, ALREADY_EXISTS, DB_ERROR };

class FriendDao : public BaseDao<model::ImFriend, int> {
public:
    FriendDao() = default;
    ~FriendDao() = default;

    // Create a new friend request
    std::pair<AddFriendResult, uint64_t> AddFriend(uint64_t user_id, uint64_t friend_id, const std::string& verify_msg);
    // Handle a friend request (accept/reject)
    std::optional<uint64_t> HandleFriend(uint64_t user_id, uint64_t sender_id, bool accept);
    // Get a friend list
    bool GetFriendList(uint64_t user_id, im::GetFriendListResp* resp);
    // Get pending friend requests
    std::vector<im::FriendReqPush> GetPendingRequests(uint64_t user_id);
};
