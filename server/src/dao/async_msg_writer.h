#pragma once

#include <atomic>
#include <thread>
#include "../core/mpsc_queue.h"
#include "message_service.pb.h"
#include "msg_scylla_dao.h"

class AsyncMsgWriter {
public:
    static AsyncMsgWriter* GetInstance() {
        static AsyncMsgWriter instance;
        return &instance;
    };

    void Start();
    void Stop();
    void Enqueue(im::P2PMessage msg);

private:
    AsyncMsgWriter() = default;
    ~AsyncMsgWriter() { Stop(); };

    void WorkerLoop();

    MPSCQueue<im::P2PMessage> queue_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    MsgScyllaDao dao_;

    static const size_t kBatchSize = 100;
};
