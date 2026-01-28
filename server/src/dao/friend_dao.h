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
    AddFriendResult add_friend(uint64_t user_id, uint64_t friend_id);
    // Handle a friend request (accept/reject)
    std::optional<uint64_t> handle_friend(uint64_t user_id, uint64_t sender_id, bool accept);
    // Get a friend list
    bool get_friend_list(uint64_t user_id, im::GetFriendListResp* resp);
};
