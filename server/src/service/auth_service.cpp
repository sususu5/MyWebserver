#include "auth_service.h"
#include "../log/log.h"

AuthService::AuthService() {}

AuthService::~AuthService() {}

auto AuthService::Register(const im::RegisterReq& req, im::RegisterResp* resp) -> void {
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

    if (user_dao_.Insert(username, password)) {
        resp->set_success(true);
        // TODO: Generate user ID
        resp->set_user_id(username);
        LOG_INFO("Register success: %s", username.c_str());
    } else {
        resp->set_success(false);
        resp->set_error_msg("Database internal error");
        LOG_ERROR("Register failed for user: %s", username.c_str());
    }
}
