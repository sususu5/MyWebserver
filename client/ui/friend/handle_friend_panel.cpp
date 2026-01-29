#include "handle_friend_panel.h"
#include <ftxui/dom/elements.hpp>
#include "../../network_manager.h"
#include "../ui_common.h"

using namespace ftxui;

HandleFriendPanel BuildHandleFriendPanel(std::string& hint,
                                         const std::function<void(uint64_t req_id, uint64_t sender_id)>& on_accept,
                                         const std::function<void(uint64_t req_id, uint64_t sender_id)>& on_reject) {
    auto btn_accept_request = Button(
        "Accept",
        [&hint, on_accept] {
            auto requests = NetworkManager::GetInstance().GetPendingFriendRequests();
            if (requests.empty()) {
                hint = "No pending requests.";
                return;
            }
            const auto& req = requests[0];
            hint = "Processing accept...";
            on_accept(req.req_id(), req.sender_id());
        },
        MakeButtonStyle());

    auto btn_reject_request = Button(
        "Reject",
        [&hint, on_reject] {
            auto requests = NetworkManager::GetInstance().GetPendingFriendRequests();
            if (requests.empty()) {
                hint = "No pending requests.";
                return;
            }
            const auto& req = requests[0];
            hint = "Processing reject...";
            on_reject(req.req_id(), req.sender_id());
        },
        MakeButtonStyle());

    auto layout = Container::Vertical({
        btn_accept_request,
        btn_reject_request,
    });

    auto renderer = Renderer(layout, [=, &hint] {
        auto requests = NetworkManager::GetInstance().GetPendingFriendRequests();

        Element content;
        if (requests.empty()) {
            content = vbox({
                text("No pending friend requests.") | center,
            });
        } else {
            auto& req = requests[0];
            std::string info = "From: " + req.sender_name() + " (ID: " + std::to_string(req.sender_id()) + ")";
            std::string msg = "Msg: " + req.verify_msg();
            std::string count_info = "Total Pending: " + std::to_string(requests.size());

            content = vbox({
                text(info),
                text(msg),
                separator(),
                text(count_info) | dim,
            });
        }

        return vbox({
                   text("Handle Friend Request") | bold,
                   separator(),
                   content | flex,
                   separator(),
                   hbox(btn_accept_request->Render() | flex, btn_reject_request->Render() | flex),
                   separator(),
                   text(hint) | color(Color::Yellow) | hcenter,
               }) |
               border | flex;
    });

    return {layout, renderer};
}
