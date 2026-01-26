#include "network_manager.h"

bool NetworkManager::Connect(const std::string& host, int port) {
    if (connected_) return true;

    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        // TODO: FTXUI Show error message
        perror("Socket creation failed");
        return false;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return false;
    }

    if (connect(sock_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return false;
    }

    connected_ = true;
    return true;
}

bool NetworkManager::SendEnvelope(const im::Envelope& env) {
    if (!connected_) return false;

    auto serialized = env.SerializeAsString();
    auto len = htonl(static_cast<uint32_t>(serialized.size()));

    std::string packet;
    packet.append(reinterpret_cast<char*>(&len), 4);
    packet.append(serialized);

    auto sent = send(sock_, packet.data(), packet.size(), 0);
    return sent == static_cast<ssize_t>(packet.size());
}

bool NetworkManager::ReceiveEnvelope(im::Envelope& env) {
    if (!connected_) return false;

    uint32_t net_len;
    auto len_buf = reinterpret_cast<char*>(&net_len);
    size_t received = 0;
    while (received < 4) {
        auto r = recv(sock_, len_buf + received, 4 - received, 0);
        if (r <= 0) return false;
        received += r;
    }

    auto msg_len = ntohl(net_len);
    std::string buffer;
    buffer.resize(msg_len);
    received = 0;
    while (received < msg_len) {
        auto r = recv(sock_, &buffer[received], msg_len - received, 0);
        if (r <= 0) return false;
        received += r;
    }

    return env.ParseFromString(buffer);
}

bool NetworkManager::Register(const std::string& username, const std::string& password, std::string& error_msg) {
    im::RegisterReq req;
    req.set_username(username);
    req.set_password(password);

    im::Envelope env;
    env.set_cmd(im::CMD_REGISTER_REQ);
    env.set_timestamp(time(NULL));
    *env.mutable_register_req() = req;

    if (!SendEnvelope(env)) {
        error_msg = "Failed to send register request";
        return false;
    }

    im::Envelope resp_env;
    if (!ReceiveEnvelope(resp_env)) {
        error_msg = "Failed to receive register response";
        return false;
    }

    if (resp_env.cmd() != im::CMD_REGISTER_RES) {
        error_msg = "Unexpected register response command";
        return false;
    }

    const auto& resp = resp_env.register_res();
    if (resp.success()) {
        if (!resp.user_id().empty()) {
            user_id_ = resp.user_id();
        }
        return true;
    } else {
        error_msg = resp.error_msg();
        return false;
    }
}

bool NetworkManager::Login(const std::string& username, const std::string& password, std::string& error_msg) {
    im::LoginReq req;
    req.set_username(username);
    req.set_password(password);

    im::Envelope env;
    env.set_cmd(im::CMD_LOGIN_REQ);
    env.set_timestamp(time(NULL));
    *env.mutable_login_req() = req;

    if (!SendEnvelope(env)) {
        error_msg = "Failed to send login request";
        return false;
    }

    im::Envelope resp_env;
    if (!ReceiveEnvelope(resp_env)) {
        error_msg = "Failed to receive login response";
        return false;
    }

    if (resp_env.cmd() != im::CMD_LOGIN_RES) {
        error_msg = "Unexpected login response command";
        return false;
    }

    const auto& resp = resp_env.login_res();
    if (resp.success()) {
        token_ = resp.token();
        if (resp.has_user_info()) {
            user_id_ = resp.user_info().user_id();
            username_ = resp.user_info().username();
        }
        return true;
    } else {
        error_msg = resp.error_msg();
        return false;
    }
}

bool NetworkManager::Logout(std::string& error_msg) {
    if (!IsLoggedIn()) {
        return true;
    }

    ClearAuth();
    Disconnect();
    return true;
}

void NetworkManager::ClearAuth() {
    token_.clear();
    user_id_.clear();
    username_.clear();
}

void NetworkManager::Disconnect() {
    if (sock_ != -1) {
        close(sock_);
        sock_ = -1;
    }
    connected_ = false;
}
