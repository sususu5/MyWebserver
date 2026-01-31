#include "chat_panel.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <vector>
#include "../../network_manager.h"
#include "../ui_common.h"

using namespace ftxui;

struct ChatState {
    int selected = 0;
    std::vector<std::string> dummy_entries;
    std::string content;
};

ChatPanel BuildChatPanel(const uint64_t& friend_id, const std::string& friend_name,
                         const std::function<void(const std::string&)>& on_send) {
    auto state = std::make_shared<ChatState>();
    auto input_content = Input(&state->content, "Type a message...");

    auto btn_send = Button(
        "Send",
        [state, on_send] {
            if (state->content.empty()) {
                return;
            }
            on_send(state->content);
            state->content.clear();
        },
        MakeButtonStyle());

    MenuOption option;
    option.entries_option.transform = [state, &friend_id](const EntryState& entry_state) {
        const auto& history = NetworkManager::GetInstance().GetP2PHistory(friend_id);
        if (entry_state.index >= (int)history.size()) {
            return text("");
        }
        const auto& msg = history[entry_state.index];
        bool is_self = msg.sender_id() == NetworkManager::GetInstance().GetUserId();
        auto msg_box = text(msg.content()) | border;

        Element e;
        if (is_self) {
            e = hbox({filler(), msg_box}) | color(Color::Green);
        } else {
            e = hbox(msg_box, filler());
        }

        if (entry_state.focused) {
            e = e | inverted;
        }
        return e;
    };

    auto msg_menu = Menu(&state->dummy_entries, &state->selected, option);

    auto btn_layout = Container::Horizontal({
        input_content,
        btn_send,
    });

    auto layout = Container::Vertical({
        msg_menu | flex,
        btn_layout,
    });

    auto renderer = Renderer(layout, [state, &friend_id, &friend_name, input_content, btn_send, msg_menu] {
        if (friend_id != 0) {
            const auto& history = NetworkManager::GetInstance().GetP2PHistory(friend_id);
            if (state->dummy_entries.size() != history.size()) {
                auto old_size = state->dummy_entries.size();
                state->dummy_entries.resize(history.size());
                // Auto-scroll to bottom if we were at the bottom or it's a new load
                if (state->selected == (int)old_size - 1 || old_size == 0) {
                    state->selected = (int)history.size() - 1;
                }
            }
        } else {
            state->dummy_entries.clear();
            state->selected = 0;
        }

        return vbox({
                   text("Chat with " + friend_name) | bold | center,
                   separator(),
                   msg_menu->Render() | vscroll_indicator | yframe | flex,
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
