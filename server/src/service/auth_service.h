#pragma once

#include "../dao/user_dao.h"
#include "../utils/uuid_generator.h"
#include "auth_service.pb.h"

class AuthService {
public:
    AuthService();
    ~AuthService();

    // Register a new user
    void user_register(const im::RegisterReq& req, im::RegisterResp* resp);
    // Login a user
    void user_login(const im::LoginReq& req, im::LoginResp* resp);

private:
    UserDao user_dao_;
    UuidGenerator uuid_generator_;
};
