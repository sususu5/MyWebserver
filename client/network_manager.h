#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdint>
#include <functional>
#include <string>
#include "protocol.pb.h"

class NetworkManager {
public:
    using OnErrorCallback = std::function<void(const std::string& error_msg)>;

    static NetworkManager& GetInstance() {
        static NetworkManager instance;
        return instance;
    }

    void SetOnErrorCallback(OnErrorCallback callback) { on_error_callback_ = callback; }

    bool Connect(const std::string& host, int port);
    // Auth Service
    bool Register(const std::string& username, const std::string& password, std::string& error_msg);
    bool Login(const std::string& username, const std::string& password, std::string& error_msg);
    bool Logout(std::string& error_msg);

    bool IsLoggedIn() const { return !token_.empty(); }
    const std::string& GetToken() const { return token_; }
    uint64_t GetUserId() const { return user_id_; }
    const std::string& GetUsername() const { return username_; }

private:
    NetworkManager() = default;
    ~NetworkManager() { Disconnect(); }

    bool SendEnvelope(const im::Envelope& env);
    bool ReceiveEnvelope(im::Envelope& env);
    void ClearAuth();
    void Disconnect();

    void ReportError(const std::string& msg) {
        if (on_error_callback_) {
            on_error_callback_(msg);
        }
    }

    int sock_ = -1;
    bool connected_ = false;
    std::string token_;
    uint64_t user_id_ = 0;
    std::string username_;
    OnErrorCallback on_error_callback_;
};
