#include "msg_scylla_dao.h"
#include <cassandra.h>
#include "../log/log.h"
#include "../pool/scylla_session.h"
#include "../utils/id_generator.h"

namespace {
std::string CassFutureError(CassFuture* future) {
    const char* message = nullptr;
    size_t message_length = 0;
    cass_future_error_message(future, &message, &message_length);
    if (message == nullptr || message_length == 0) {
        return "unknown error";
    }
    return std::string(message, message_length);
}
}  // namespace

bool MsgScyllaDao::InsertMessage(const std::string& conversation_id, const im::P2PMessage& msg) {
    auto* session = ScyllaSession::Instance()->Session();
    if (!session) {
        LOG_ERROR("Scylla session is not initialized");
        return false;
    }

    constexpr const char* k_insert_query =
        "INSERT INTO im.messages (conversation_id, timestamp, message_id, sender_id, "
        "receiver_id, content_type, content) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    CassStatement* statement = cass_statement_new(k_insert_query, 7);
    auto p2p_conv_id = IdGenerator::GenerateP2PConvId(msg.sender_id(), msg.receiver_id());
    cass_statement_bind_string(statement, 0, p2p_conv_id.c_str());
    cass_statement_bind_int64(statement, 1, static_cast<cass_int64_t>(msg.timestamp()));
    cass_statement_bind_string(statement, 2, msg.msg_id().c_str());
    cass_statement_bind_string(statement, 3, msg.sender_id().c_str());
    cass_statement_bind_string(statement, 4, msg.receiver_id().c_str());
    cass_statement_bind_int32(statement, 5, static_cast<cass_int32_t>(msg.content_type()));
    const std::string& content = msg.content();
    cass_statement_bind_bytes(statement, 6, reinterpret_cast<const cass_byte_t*>(content.data()), content.size());

    CassFuture* future = cass_session_execute(session, statement);
    cass_future_wait(future);

    bool ok = true;
    if (cass_future_error_code(future) != CASS_OK) {
        LOG_ERROR("Scylla insert failed: {}", CassFutureError(future));
        ok = false;
    }

    cass_future_free(future);
    cass_statement_free(statement);
    return ok;
}
