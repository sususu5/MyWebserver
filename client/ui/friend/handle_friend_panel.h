#pragma once

#include <ftxui/component/component.hpp>
#include <functional>
#include <string>

struct HandleFriendPanel {
    ftxui::Component layout;
    ftxui::Component renderer;
};

HandleFriendPanel BuildHandleFriendPanel(std::string& pending_request, std::string& hint,
                                         const std::function<void()>& on_accept,
                                         const std::function<void()>& on_reject);
