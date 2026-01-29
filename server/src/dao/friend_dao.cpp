#include "friend_dao.h"
#include <sqlpp11/mysql/connection.h>
#include <sqlpp11/sqlpp11.h>
#include "friend_service.pb.h"
#include "model/schema.h"

namespace {
const int STATUS_PENDING = 0;
const int STATUS_ACCEPTED = 1;
const int STATUS_REJECTED = 2;
}  // namespace

AddFriendResult FriendDao::add_friend(uint64_t sender_id, uint64_t receiver_id) {
    return execute(
        [&](auto& conn) {
            auto exists_query = conn(sqlpp::select(table_.id)
                                         .from(table_)
                                         .where((table_.userId == sender_id) && (table_.friendId == receiver_id))
                                         .limit(1u));
            if (!exists_query.empty()) {
                return AddFriendResult::ALREADY_EXISTS;
            }

            conn(sqlpp::insert_into(table_).set(table_.userId = sender_id, table_.friendId = receiver_id,
                                                table_.status = STATUS_PENDING));
            return AddFriendResult::SUCCESS;
        },
        "AddFriend", AddFriendResult::DB_ERROR);
}

std::optional<uint64_t> FriendDao::handle_friend(uint64_t receiver_id, uint64_t sender_id, bool accept) {
    return execute(
        [&](auto& conn) -> std::optional<uint64_t> {
            // Start a transaction, all operations in the transaction will be committed or rolled back together
            conn.start_transaction();
            try {
                if (accept) {
                    conn(sqlpp::update(table_)
                             .set(table_.status = STATUS_ACCEPTED)
                             .where((table_.userId == sender_id) && (table_.friendId == receiver_id)));

                    // Check if reverse relation exists
                    auto reverse_exists = conn(sqlpp::select(table_.id).from(table_).where(
                        (table_.userId == receiver_id) && (table_.friendId == sender_id)));

                    if (reverse_exists.empty()) {
                        conn(sqlpp::insert_into(table_).set(table_.userId = receiver_id, table_.friendId = sender_id,
                                                            table_.status = STATUS_ACCEPTED));
                    } else {
                        conn(sqlpp::update(table_)
                                 .set(table_.status = STATUS_ACCEPTED)
                                 .where((table_.userId == receiver_id) && (table_.friendId == sender_id)));
                    }
                } else {
                    conn(sqlpp::update(table_)
                             .set(table_.status = STATUS_REJECTED)
                             .where((table_.userId == sender_id) && (table_.friendId == receiver_id)));
                }
                conn.commit_transaction();
                return sender_id;
            } catch (const std::exception& e) {
                conn.rollback_transaction(false);
                throw;
            }
        },
        "HandleFriend", std::optional<uint64_t>{});
}

bool FriendDao::get_friend_list(uint64_t user_id, im::GetFriendListResp* resp) {
    return execute_bool(
        [&](auto& conn) {
            model::ImUser user_table;
            auto result = conn(sqlpp::select(user_table.userId, user_table.username)
                                   .from(table_.join(user_table).on(table_.friendId == user_table.userId))
                                   .where((table_.userId == user_id) && (table_.status == STATUS_ACCEPTED)));

            for (const auto& row : result) {
                auto* friend_info = resp->add_friend_list();
                friend_info->set_user_id(row.userId);
                friend_info->set_username(row.username);
                friend_info->set_status(im::UserStatus::USER_STATUS_ONLINE);
            }
            return true;
        },
        "GetFriendList");
}
