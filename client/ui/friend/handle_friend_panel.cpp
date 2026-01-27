#include "handle_friend_panel.h"
#include <ftxui/dom/elements.hpp>
#include "../ui_common.h"

using namespace ftxui;

HandleFriendPanel BuildHandleFriendPanel(std::string& pending_request, std::string& hint,
                                         const std::function<void()>& on_accept,
                                         const std::function<void()>& on_reject) {
    auto btn_accept_request = Button(
        "Accept",
        [&] {
            hint = "Request accepted.";
            pending_request.clear();
            on_accept();
        },
        MakeButtonStyle());
    auto btn_reject_request = Button(
        "Reject",
        [&] {
            hint = "Request rejected.";
            pending_request.clear();
            on_reject();
        },
        MakeButtonStyle());

    auto layout = Container::Vertical({
        btn_accept_request,
        btn_reject_request,
    });

    auto renderer = Renderer(layout, [=, &pending_request, &hint] {
        const auto& request_text =
            pending_request.empty() ? std::string("No pending friend requests.") : pending_request;
        return vbox({
                   text("Friend Requests") | bold,
                   separator(),
                   paragraph(request_text) | flex,
                   separator(),
                   hbox(btn_accept_request->Render() | flex, btn_reject_request->Render() | flex),
                   separator(),
                   text(hint) | color(Color::Yellow) | hcenter,
               }) |
               border | flex;
    });

    return {layout, renderer};
}
