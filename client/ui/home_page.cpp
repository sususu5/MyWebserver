#include "home_page.h"
#include <ftxui/dom/elements.hpp>
#include "ui_common.h"

using namespace ftxui;

HomePage BuildHomePage(const std::function<void()>& on_logout) {
    auto btn_logout = Button("Logout", on_logout, MakeButtonStyle());
    auto main_layout = Container::Vertical({
        btn_logout,
    });

    auto main_renderer = Renderer(main_layout, [&, btn_logout] {
        return vbox({
            hbox({
                text("Main Page") | bold,
                filler(),
                btn_logout->Render(),
            }) | border,
            text("Welcome to IM System!") | center | flex,
        });
    });

    return {main_layout, main_renderer};
}
