#pragma once

#include "../dao/user_dao.h"
#include "auth_service.pb.h"

class AuthService {
public:
    AuthService();
    ~AuthService();

    // Register a new user
    auto Register(const im::RegisterReq& req, im::RegisterResp* resp) -> void;

private:
    UserDao user_dao_;
};
