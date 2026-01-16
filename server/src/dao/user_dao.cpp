#include "user_dao.h"
#include <sqlpp11/mysql/connection.h>
#include <sqlpp11/sqlpp11.h>

UserDao::UserDao() = default;

UserDao::~UserDao() = default;

auto UserDao::QueryExist(const std::string& username) -> bool {
    return exists("QueryExist", table_.username == username);
}

auto UserDao::Insert(const std::string& user_id, const std::string& username, const std::string& password) -> bool {
    return insert("Insert", table_.userId = user_id, table_.username = username, table_.password = password);
}

auto UserDao::VerifyUser(const std::string& username, const std::string& password) -> bool {
    return execute_bool(
        [&](auto& conn) {
            auto result = conn(sqlpp::select(table_.password).from(table_).where(table_.username == username));
            if (result.empty()) return false;
            return result.front().password == password;
        },
        "VerifyUser");
}

im::User UserDao::FindByUsername(const std::string& username) {
    return execute(
        [&](auto& conn) -> im::User {
            auto result =
                conn(sqlpp::select(table_.userId, table_.username).from(table_).where(table_.username == username));
            im::User user;
            if (!result.empty()) {
                const auto& row = result.front();
                user.set_user_id(row.userId);
                user.set_username(row.username);
                user.set_status(im::UserStatus::USER_STATUS_ONLINE);
            }
            return user;
        },
        "FindByUsername", im::User{});
}
