#pragma once

#include "../core/tcp_connection.h"
#include "../dao/user_dao.h"
#include "auth_service.pb.h"

class AuthService {
public:
    AuthService() = default;
    ~AuthService() = default;

    // Register a new user
    void user_register(const im::RegisterReq& req, im::RegisterResp* resp);
    // Login a user
    void user_login(TcpConnection* conn, const im::LoginReq& req, im::LoginResp* resp);

private:
    UserDao user_dao_;
};
