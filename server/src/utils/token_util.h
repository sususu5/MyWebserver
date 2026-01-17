#pragma once

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <chrono>
#include <nlohmann/json.hpp>
#include <string>
#include "../log/log.h"

using traits = jwt::traits::nlohmann_json;

class TokenUtil {
public:
    static std::string create_token(const std::string& user_id, const std::string& username) {
        auto now = std::chrono::system_clock::now();
        auto expires_at = now + std::chrono::hours(24);
        const std::string kSecretKey = "yzyxjj20021225";

        auto token = jwt::create<traits>()
                         .set_issuer("mywebserver")
                         .set_type("JWS")
                         .set_payload_claim("user_id", user_id)
                         .set_payload_claim("username", username)
                         .set_issued_at(now)
                         .set_expires_at(expires_at)
                         .sign(jwt::algorithm::hs256{kSecretKey});

        return token;
    }

    static bool verify_token(const std::string& token, std::string& out_user_id) {
        const std::string kSecretKey = "yzyxjj20021225";

        try {
            auto decoded = jwt::decode<traits>(token);
            auto verifier =
                jwt::verify<traits>().allow_algorithm(jwt::algorithm::hs256{kSecretKey}).with_issuer("mywebserver");

            verifier.verify(decoded);

            if (decoded.has_payload_claim("user_id")) {
                out_user_id = decoded.get_payload_claim("user_id").as_string();
                return true;
            }
            return false;
        } catch (const std::exception& e) {
            LOG_WARN("Token verification failed: {}", e.what());
            return false;
        }
    }
};
