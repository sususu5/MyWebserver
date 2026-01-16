#pragma once

#include <string>
#include "base_dao.h"
#include "common.pb.h"
#include "model/schema.h"

class UserDao : public BaseDao<model::ImUser, std::string> {
public:
    UserDao();
    ~UserDao();

    // Check if user exists
    bool QueryExist(const std::string& username);
    // Insert a new user
    bool Insert(const std::string& user_id, const std::string& username, const std::string& password);
    // Verify user credentials
    bool VerifyUser(const std::string& username, const std::string& password);
    // Find user by username
    im::User FindByUsername(const std::string& username);
};
