#pragma once

#include <cstdint>
#include <ftxui/component/component.hpp>
#include <functional>
#include <string>

struct ChatPanel {
    ftxui::Component layout;
    ftxui::Component renderer;
};

ChatPanel BuildChatPanel(const uint64_t& friend_id, const std::string& friend_name,
                         const std::function<void(const std::string&)>& on_send);
