#pragma once

#include <optional>
#include <string>
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
    AddFriendResult add_friend(const std::string& user_id, const std::string& friend_id);
    // Handle a friend request (accept/reject)
    std::optional<std::string> handle_friend(const std::string& user_id, const std::string& sender_id, bool accept);
    // Get a friend list
    bool get_friend_list(const std::string& user_id, im::GetFriendListResp* resp);
};
