#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <functional>
#include <string>

struct HomePageState {
    std::string add_friend_user_id;
    std::string add_friend_verify_msg;
    std::string add_friend_hint;
    std::string handle_req_id;
    std::string handle_sender_id;
    std::string handle_hint;
    std::vector<std::string> friend_names;
    int current_panel = 0;
};

struct HomePage {
    ftxui::Component layout;
    ftxui::Component renderer;
};

HomePage BuildHomePage(const std::function<void()>& on_logout, HomePageState* state);
