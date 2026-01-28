#include "handle_friend_panel.h"
#include <ftxui/dom/elements.hpp>
#include "../ui_common.h"

using namespace ftxui;

HandleFriendPanel BuildHandleFriendPanel(std::string& req_id, std::string& sender_id, std::string& hint,
                                         const std::function<void()>& on_accept,
                                         const std::function<void()>& on_reject) {
    auto input_req_id = Input(&req_id, "Request ID");
    auto input_sender_id = Input(&sender_id, "Sender ID");

    auto btn_accept_request = Button(
        "Accept",
        [&] {
            if (req_id.empty() || sender_id.empty()) {
                hint = "IDs cannot be empty.";
                return;
            }
            hint = "Processing accept...";
            on_accept();
        },
        MakeButtonStyle());
    auto btn_reject_request = Button(
        "Reject",
        [&] {
            if (req_id.empty() || sender_id.empty()) {
                hint = "IDs cannot be empty.";
                return;
            }
            hint = "Processing reject...";
            on_reject();
        },
        MakeButtonStyle());

    auto layout = Container::Vertical({
        input_req_id,
        input_sender_id,
        btn_accept_request,
        btn_reject_request,
    });

    auto renderer = Renderer(layout, [=, &hint] {
        return vbox({
                   text("Handle Friend Request") | bold,
                   separator(),
                   hbox(text("Req ID:    "), input_req_id->Render()) | flex,
                   hbox(text("Sender ID: "), input_sender_id->Render()) | flex,
                   separator(),
                   hbox(btn_accept_request->Render() | flex, btn_reject_request->Render() | flex),
                   separator(),
                   text(hint) | color(Color::Yellow) | hcenter,
               }) |
               border | flex;
    });

    return {layout, renderer};
}
