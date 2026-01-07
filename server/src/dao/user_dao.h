#pragma once

#include <string>
#include "../pool/sqlconnpool.h"

class UserDao {
public:
    UserDao();
    ~UserDao();

    auto QueryExist(const std::string& username) -> bool;
    auto Insert(const std::string& username, const std::string& password) -> bool;
    auto VerifyUser(const std::string& username, const std::string& password) -> bool;
};
