#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "protocol.pb.h"

class NetworkManager {
public:
    using OnErrorCallback = std::function<void(const std::string& error_msg)>;
    using OnFriendRequestCallback = std::function<void(const im::FriendReqPush& req)>;
    using OnFriendStatusCallback = std::function<void(const im::FriendStatusPush& status)>;

    static NetworkManager& GetInstance() {
        static NetworkManager instance;
        return instance;
    }

    bool Connect(const std::string& host, int port);
    void SetOnErrorCallback(OnErrorCallback callback) { on_error_callback_ = callback; }
    void SetOnFriendRequestCallback(OnFriendRequestCallback callback) { on_friend_request_callback_ = callback; }
    void SetOnFriendStatusCallback(OnFriendStatusCallback callback) { on_friend_status_callback_ = callback; }

    // Auth Service
    bool Register(const std::string& username, const std::string& password, std::string& error_msg);
    bool Login(const std::string& username, const std::string& password, std::string& error_msg);
    bool Logout(std::string& error_msg);

    // Friend Service
    bool AddFriend(uint64_t receiver_id, const std::string& verify_msg, std::string& error_msg);
    bool HandleFriendRequest(uint64_t req_id, uint64_t sender_id, im::FriendAction action, std::string& error_msg);
    bool GetFriendList(std::vector<im::User>& friend_info_list, std::string& error_msg);

    bool IsLoggedIn() const { return !token_.empty(); }
    const std::string& GetToken() const { return token_; }
    uint64_t GetUserId() const { return user_id_; }
    const std::string& GetUsername() const { return username_; }

    std::vector<im::FriendReqPush> GetPendingFriendRequests();
    void RemovePendingRequest(uint64_t req_id);

private:
    NetworkManager() = default;
    ~NetworkManager() { Disconnect(); }

    // Internal Helpers
    bool SendEnvelope(const im::Envelope& env);
    bool SendRequestAndWait(const im::Envelope& request, im::Envelope& response, im::CommandType expected_cmd);
    void ListenerLoop();
    void HeartbeatLoop();
    void ClearAuth();
    void Disconnect();

    void ReportError(const std::string& msg) {
        if (on_error_callback_) {
            on_error_callback_(msg);
        }
    }

    // Network State
    int sock_ = -1;
    std::atomic<bool> connected_{false};
    std::string token_;
    uint64_t user_id_ = 0;
    std::string username_;

    // Async Handling
    std::thread listener_thread_;
    std::thread heartbeat_thread_;
    std::atomic<bool> running_{false};

    std::mutex mutex_;
    std::condition_variable cv_response_;
    im::Envelope response_envelope_;
    bool has_response_ = false;

    // Callbacks & Storage
    OnErrorCallback on_error_callback_;
    OnFriendRequestCallback on_friend_request_callback_;
    OnFriendStatusCallback on_friend_status_callback_;
    std::vector<im::FriendReqPush> pending_friend_requests_;
};
