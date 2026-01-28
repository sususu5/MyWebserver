#pragma once

#include <ftxui/component/component.hpp>
#include <functional>
#include <string>

struct HandleFriendPanel {
    ftxui::Component layout;
    ftxui::Component renderer;
};

HandleFriendPanel BuildHandleFriendPanel(std::string& req_id, std::string& sender_id, std::string& hint,
                                         const std::function<void()>& on_accept,
                                         const std::function<void()>& on_reject);
