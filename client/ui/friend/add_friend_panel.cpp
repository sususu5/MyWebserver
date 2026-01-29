#include "add_friend_panel.h"
#include <ftxui/dom/elements.hpp>
#include "../ui_common.h"

using namespace ftxui;

AddFriendPanel BuildAddFriendPanel(std::string& user_id, std::string& verify_msg, std::string& hint,
                                   const std::function<void()>& on_send, const std::function<void()>& on_cancel) {
    auto input_user_id = Input(&user_id, "User ID");
    auto input_verify_msg = Input(&verify_msg, "Verification");
    auto btn_send_request = Button(
        "Send",
        [&user_id, &hint, on_send] {
            if (user_id.empty()) {
                hint = "User ID cannot be empty.";
                return;
            }
            hint = "Request prepared. Please connect to backend.";
            on_send();
        },
        MakeButtonStyle());
    auto btn_cancel_request = Button(
        "Cancel",
        [&user_id, &verify_msg, &hint, on_cancel] {
            user_id.clear();
            verify_msg.clear();
            hint.clear();
            on_cancel();
        },
        MakeButtonStyle());

    auto layout = Container::Vertical({
        input_user_id,
        input_verify_msg,
        btn_send_request,
        btn_cancel_request,
    });

    // Explicitly capture components to ensure they are available in the renderer
    auto renderer = Renderer(layout, [=, &hint] {
        return vbox({
                   text("Add Friend") | bold,
                   separator(),
                   hbox(text("User ID: "), input_user_id->Render()) | flex,
                   hbox(text("Verify: "), input_verify_msg->Render()) | flex,
                   separator(),
                   hbox(btn_send_request->Render() | flex, btn_cancel_request->Render() | flex),
                   separator(),
                   text(hint) | color(Color::Yellow) | hcenter,
               }) |
               border | flex;
    });

    return {layout, renderer};
}