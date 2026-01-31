#include "chat_panel.h"
#include <ftxui/dom/elements.hpp>
#include "../../network_manager.h"
#include "../ui_common.h"

using namespace ftxui;

ChatPanel BuildChatPanel(const uint64_t& friend_id, const std::string& friend_name,
                         const std::function<void(const std::string&)>& on_send) {
    static std::string content;
    auto input_content = Input(&content, "Type a message...");
    auto btn_send = Button(
        "Send",
        [&, on_send] {
            if (content.empty()) {
                return;
            }
            on_send(content);
            content.clear();
        },
        MakeButtonStyle());
    auto btn_layout = Container::Horizontal({
        input_content,
        btn_send,
    });
    auto layout = Container::Vertical({
        btn_layout,
    });

    auto renderer = Renderer(layout, [&friend_id, &friend_name, input_content, btn_send] {
        Elements msgs;
        if (friend_id != 0) {
            const auto& history = NetworkManager::GetInstance().GetP2PHistory(friend_id);
            for (const auto& msg : history) {
                bool is_self = msg.sender_id() == NetworkManager::GetInstance().GetUserId();
                auto msg_box = text(msg.content()) | border;
                if (is_self) {
                    msgs.push_back(hbox({filler(), msg_box}) | color(Color::Green));
                } else {
                    msgs.push_back(hbox(msg_box, filler()));
                }
            }
        }

        return vbox({
                   text("Chat with " + friend_name) | bold | center,
                   separator(),
                   vbox(msgs) | flex | yframe,
                   separator(),
                   hbox({
                       input_content->Render() | flex,
                       btn_send->Render(),
                   }),
               }) |
               border | flex;
    });

    return {layout, renderer};
}
