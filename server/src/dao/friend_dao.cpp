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

std::pair<AddFriendResult, uint64_t> FriendDao::AddFriend(uint64_t sender_id, uint64_t receiver_id,
                                                          const std::string& verify_msg) {
    return execute(
        [&](auto& conn) -> std::pair<AddFriendResult, uint64_t> {
            auto exists_query = conn(sqlpp::select(table_.id)
                                         .from(table_)
                                         .where((table_.userId == sender_id) && (table_.friendId == receiver_id))
                                         .limit(1u));
            if (!exists_query.empty()) {
                return {AddFriendResult::ALREADY_EXISTS, 0};
            }

            uint64_t last_id =
                conn(sqlpp::insert_into(table_).set(table_.userId = sender_id, table_.friendId = receiver_id,
                                                    table_.status = STATUS_PENDING, table_.verifyMsg = verify_msg));
            return {AddFriendResult::SUCCESS, last_id};
        },
        "AddFriend", std::make_pair(AddFriendResult::DB_ERROR, 0));
}

std::optional<uint64_t> FriendDao::HandleFriend(uint64_t receiver_id, uint64_t sender_id, bool accept) {
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

bool FriendDao::GetFriendList(uint64_t user_id, im::GetFriendListResp* resp) {
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

std::vector<im::FriendReqPush> FriendDao::GetPendingRequests(uint64_t user_id) {
    return execute(
        [&](auto& conn) {
            model::ImUser user_table;
            // table_ is im_friend (the request), user_table is im_user (the sender)
            auto result =
                conn(sqlpp::select(table_.id, table_.userId, user_table.username, table_.verifyMsg, table_.createdAt)
                         .from(table_.inner_join(user_table).on(table_.userId == user_table.userId))
                         .where((table_.friendId == user_id) && (table_.status == STATUS_PENDING)));

            std::vector<im::FriendReqPush> reqs;
            for (const auto& row : result) {
                im::FriendReqPush req;
                req.set_req_id(row.id);
                req.set_sender_id(row.userId);
                req.set_sender_name(row.username);
                req.set_verify_msg(row.verifyMsg);
                req.set_timestamp(std::chrono::system_clock::to_time_t(
                    std::chrono::time_point_cast<std::chrono::system_clock::duration>(row.createdAt.value())));

                reqs.push_back(req);
            }
            return reqs;
        },
        "GetPendingRequests", std::vector<im::FriendReqPush>{});
}
