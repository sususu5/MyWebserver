#include "ui_common.h"

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
