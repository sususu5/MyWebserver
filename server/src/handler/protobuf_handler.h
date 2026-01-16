#pragma once

#include <functional>
#include <unordered_map>
#include "../service/auth_service.h"
#include "message.pb.h"
#include "tcp_connection.h"

/**
 * ProtobufHandler - Handles binary protobuf protocol over TCP
 *
 * Wire format: [4-byte length (big-endian)] [protobuf payload]
 *
 * The handler maintains a simple state machine:
 * 1. Read length prefix (4 bytes)
 * 2. Read payload (length bytes)
 * 3. Parse and dispatch based on CommandType
 * 4. Send response with same wire format
 */
class ProtobufHandler : public ProtocolHandler {
public:
    explicit ProtobufHandler(AuthService* auth_service);
    ~ProtobufHandler() override = default;

    bool process(Buffer& read_buff, Buffer& write_buff) override;
    bool is_keep_alive() const override { return true; }

private:
    // Message codec
    bool try_decode_message(Buffer& read_buff, im::Envelope& envelope);
    void encode_message(const im::Envelope& envelope, Buffer& write_buff);

    // Command dispatcher
    void dispatch(const im::Envelope& request, im::Envelope& response);

    // Command handlers
    void handle_register(const im::Envelope& request, im::Envelope& response);
    void handle_unknown(const im::Envelope& request, im::Envelope& response);

    // Services (injected dependencies)
    AuthService* auth_service_;

    // Constants
    static constexpr size_t kHeaderSize = 4;            // Length prefix size
    static constexpr size_t kMaxMessageSize = 1 << 20;  // 1MB max message size
};

// Specialization for std::format to handle im::CommandType directly
template <>
struct std::formatter<im::CommandType> : std::formatter<int> {
    auto format(im::CommandType cmd, format_context& ctx) const {
        return formatter<int>::format(static_cast<int>(cmd), ctx);
    }
};
