#pragma once

#include <ftxui/component/component.hpp>
#include <functional>
#include <string>

struct HandleFriendPanel {
    ftxui::Component layout;
    ftxui::Component renderer;
};

HandleFriendPanel BuildHandleFriendPanel(std::string& hint,
                                         const std::function<void(uint64_t req_id, uint64_t sender_id)>& on_accept,
                                         const std::function<void(uint64_t req_id, uint64_t sender_id)>& on_reject);
