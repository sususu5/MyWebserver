#include "auth_service.h"
#include "../log/log.h"
#include "../utils/uuid_generator.h"

AuthService::AuthService() {}

AuthService::~AuthService() {}

void AuthService::user_register(const im::RegisterReq& req, im::RegisterResp* resp) {
    const auto& username = req.username();
    const auto& password = req.password();

    if (username.empty() || password.empty()) {
        resp->set_success(false);
        resp->set_error_msg("Username or password cannot be empty");
        return;
    }

    if (user_dao_.QueryExist(username)) {
        resp->set_success(false);
        resp->set_error_msg("Username already exists");
        return;
    }

    const auto& user_id = uuid_generator_.generate();
    if (user_dao_.Insert(user_id, username, password)) {
        resp->set_success(true);
        resp->set_user_id(user_id);
        LOG_INFO("Register success: {} (ID: {})", username, user_id);
    } else {
        resp->set_success(false);
        resp->set_error_msg("Database internal error");
        LOG_ERROR("Register failed for user: {}", username);
    }
}

void AuthService::user_login(const im::LoginReq& req, im::LoginResp* resp) {
    const auto& username = req.username();
    const auto& password = req.password();

    if (username.empty() || password.empty()) {
        resp->set_success(false);
        resp->set_error_msg("Username or password cannot be empty");
        return;
    }

    if (!user_dao_.QueryExist(username)) {
        resp->set_success(false);
        resp->set_error_msg("Username not found");
        return;
    }

    if (user_dao_.VerifyUser(username, password)) {
        resp->set_success(true);
        *resp->mutable_user_info() = user_dao_.FindByUsername(username);
        LOG_INFO("Login success: {}", username);
    } else {
        resp->set_success(false);
        resp->set_error_msg("Database internal error");
        LOG_ERROR("Login failed for user: {}", username);
    }
}
