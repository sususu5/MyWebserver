#pragma once

#include <ftxui/component/component.hpp>
#include <functional>
#include <string>

struct AddFriendPanel {
    ftxui::Component layout;
    ftxui::Component renderer;
};

AddFriendPanel BuildAddFriendPanel(std::string& user_id, std::string& verify_msg, std::string& hint,
                                   const std::function<void()>& on_send, const std::function<void()>& on_cancel);
