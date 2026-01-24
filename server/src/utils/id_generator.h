#pragma once

#include <random>
#include <sstream>
#include <string>

class IdGenerator {
public:
    // generate a random UUID v4 string
    static std::string GenerateUuid() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);

        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        }
        return ss.str();
    }

    static std::string GenerateP2PConvId(const std::string& sender_id, const std::string& receiver_id) {
        std::string small = std::min(sender_id, receiver_id);
        std::string large = std::max(sender_id, receiver_id);

        std::stringstream ss;
        ss << "p2p_" << small << "_" << large;
        return ss.str();
    }
};
