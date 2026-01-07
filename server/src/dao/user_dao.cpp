#include "user_dao.h"
#include <sqlpp11/mysql/connection.h>
#include <sqlpp11/sqlpp11.h>
#include "schema.h"

using namespace model;

UserDao::UserDao() {}

UserDao::~UserDao() {}

auto UserDao::QueryExist(const std::string& username) -> bool {
    sqlpp::mysql::connection* sql = nullptr;
    SqlConnRAII conn(&sql, SqlConnPool::Instance());
    if (!sql) {
        LOG_ERROR("Get Database Connection failed!");
        return false;
    }

    try {
        const auto& user = User{};
        auto result = (*sql)(select(user.username).from(user).where(user.username == username));
        return !result.empty();
    } catch (const std::exception& e) {
        LOG_ERROR("QueryExist error: %s", e.what());
        return false;
    }
}

auto UserDao::Insert(const std::string& username, const std::string& password) -> bool {
    sqlpp::mysql::connection* sql = nullptr;
    SqlConnRAII conn(&sql, SqlConnPool::Instance());
    if (!sql) {
        LOG_ERROR("Get Database Connection failed!");
        return false;
    }

    try {
        const auto& user = User{};
        (*sql)(insert_into(user).set(user.username = username, user.password = password));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Insert error: %s", e.what());
        return false;
    }
}

auto UserDao::VerifyUser(const std::string& username, const std::string& password) -> bool {
    sqlpp::mysql::connection* sql = nullptr;
    SqlConnRAII conn(&sql, SqlConnPool::Instance());
    if (!sql) {
        LOG_ERROR("Get Database Connection failed!");
        return false;
    }

    try {
        const auto& user = User{};
        auto result = (*sql)(select(user.password).from(user).where(user.username == username));
        if (result.empty()) return false;
        const auto& row = result.front();
        return row.password == password;
    } catch (const std::exception& e) {
        LOG_ERROR("VerifyUser error: %s", e.what());
        return false;
    }
}