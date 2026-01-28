#pragma once

#include <chrono>
#include <cstdint>
#include <random>
#include <string>

class IdGenerator {
public:
    // generate a random 64-bit integer id
    static uint64_t GenerateRandId() {
        constexpr uint64_t EPOCH = 1704067200000ULL;

        uint64_t now_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();

        if (now_ms < EPOCH) {
            now_ms = EPOCH;
        }

        thread_local std::mt19937 generator(std::random_device{}());
        thread_local std::uniform_int_distribution<uint32_t> distribution(0, 0x3FFFFF);
        uint64_t random_val = distribution(generator);

        return ((now_ms - EPOCH) << 22) | random_val;
    }

    static std::string GenerateP2PConvId(uint64_t sender_id, uint64_t receiver_id) {
        if (sender_id <= receiver_id) {
            return std::to_string(sender_id) + "_" + std::to_string(receiver_id);
        } else {
            return std::to_string(receiver_id) + "_" + std::to_string(sender_id);
        }
    }
};
