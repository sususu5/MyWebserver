#pragma once

#include <cstdint>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <functional>
#include <string>
#include <vector>
#include "protocol.pb.h"

struct HomePageState {
    // Add friend panel state
    std::string add_friend_user_id;
    std::string add_friend_verify_msg;
    std::string add_friend_hint;
    std::string handle_req_id;
    std::string handle_sender_id;
    std::string handle_hint;

    // Chat panel state
    std::vector<std::string> friend_names;
    int selected_friend_index = 0;
    uint64_t current_chat_friend_id = 0;
    std::string current_chat_friend_name;
    std::vector<im::User> friend_list;

    int current_panel = 0;
};

struct HomePage {
    ftxui::Component layout;
    ftxui::Component renderer;
};

HomePage BuildHomePage(const std::function<void()>& on_logout, HomePageState* state);
