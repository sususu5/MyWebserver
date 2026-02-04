#include "async_msg_writer.h"
#include <chrono>
#include <iterator>
#include "../log/log.h"

void AsyncMsgWriter::Start() {
    bool expected = false;
    if (running_.compare_exchange_strong(expected, true)) {
        worker_ = std::thread(&AsyncMsgWriter::WorkerLoop, this);
        LOG_INFO("AsyncMsgWriter started.");
    }
}

void AsyncMsgWriter::Stop() {
    bool expected = true;
    if (running_.compare_exchange_strong(expected, false)) {
        if (worker_.joinable()) {
            worker_.join();
        }
        LOG_INFO("AsyncMsgWriter stopped.");
    }
}

void AsyncMsgWriter::Enqueue(im::P2PMessage msg) { queue_.enqueue(std::move(msg)); }

void AsyncMsgWriter::WorkerLoop() {
    std::vector<im::P2PMessage> batch_buffer;
    batch_buffer.reserve(kBatchSize);

    const int kMaxRetries = 3;
    const int kBaseWaitMs = 50;
    const int kMaxWaitMs = 1000;

    LOG_INFO("AsyncMsgWriter WorkerLoop started.");

    while (running_) {
        auto count = queue_.dequeue_bulk(std::back_inserter(batch_buffer), kBatchSize);

        if (count > 0) {
            bool success = false;
            int retry_count = 0;
            while (retry_count <= kMaxRetries) {
                if (dao_.InsertBatch(batch_buffer)) {
                    success = true;
                    break;
                }
                retry_count++;
                if (retry_count > kMaxRetries) break;
                auto wait_ms = kBaseWaitMs * (1 << (retry_count - 1));
                if (wait_ms > kMaxWaitMs) wait_ms = kMaxWaitMs;
                LOG_WARN("Batch insert failed, retrying {}/{} in {}ms...", retry_count, kMaxRetries, wait_ms);
                std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
            }

            if (!success) {
                LOG_ERROR("Failed to insert batch of {} messages.", count);
            }
            batch_buffer.clear();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    while (!queue_.empty()) {
        auto count = queue_.dequeue_bulk(std::back_inserter(batch_buffer), kBatchSize);
        if (count > 0) {
            if (!dao_.InsertBatch(batch_buffer)) {
                LOG_ERROR("Failed to insert batch of {} messages.", count);
            }
            batch_buffer.clear();
        }
    }

    LOG_INFO("AsyncMsgWriter WorkerLoop stopped. Flushed remaining messages.");
}
