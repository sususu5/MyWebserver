#include "ui_common.h"
#include <ftxui/component/event.hpp>

using namespace ftxui;

ButtonOption MakeButtonStyle() {
    auto opt = ButtonOption::Animated();
    opt.transform = [](const EntryState& state) {
        auto content = text(state.label) | center;
        auto bg = state.focused ? Color::GrayLight : Color::GrayDark;
        auto fg = state.focused ? Color::Black : Color::White;
        return content | flex | bgcolor(bg) | color(fg);
    };
    return opt;
}

InputOption MakePasswordOption() {
    InputOption opt;
    opt.password = true;
    return opt;
}

Component Modal(Component main, Component modal, bool* show_modal) {
    auto container = Container::Vertical({
        main,
        modal,
    });

    auto renderer = Renderer(container, [=] {
        auto doc = main->Render();
        if (*show_modal) {
            doc = dbox({
                doc,
                modal->Render() | clear_under | center,
            });
        }
        return doc;
    });

    return CatchEvent(renderer, [=](Event event) {
        if (*show_modal) {
            return modal->OnEvent(event);
        }
        return main->OnEvent(event);
    });
}

Component ErrorModal(const std::string* error_msg, std::function<void()> on_close) {
    auto close_button = Button("Close", on_close, MakeButtonStyle());
    return Renderer(close_button, [=] {
        return window(text("Error") | color(Color::Red) | bold, vbox({
                                                                    text(*error_msg) | center,
                                                                    separator(),
                                                                    close_button->Render() | center,
                                                                })) |
               size(WIDTH, GREATER_THAN, 40);
    });
}
