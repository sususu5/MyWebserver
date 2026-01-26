#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include "protocol.pb.h"

class NetworkManager {
public:
    static NetworkManager& GetInstance() {
        static NetworkManager instance;
        return instance;
    }

    bool Connect(const std::string& host, int port);
    bool Register(const std::string& username, const std::string& password, std::string& error_msg);
    // bool Login(...)

private:
    NetworkManager() = default;
    ~NetworkManager();

    int sock_ = -1;
    bool connected_ = false;

    bool SendEnvelope(const im::Envelope& env);
    bool ReceiveEnvelope(im::Envelope& env);
};
