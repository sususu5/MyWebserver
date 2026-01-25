#include "auth_page.h"
#include <ftxui/dom/elements.hpp>
#include "../ui_common.h"

using namespace ftxui;

AuthPage BuildAuthPage(const std::function<void()>& on_register, const std::function<void()>& on_login,
                       const std::function<void()>& on_quit) {
    auto btn_auth_register = Button("Register", on_register, MakeButtonStyle());
    auto btn_auth_login = Button("Login", on_login, MakeButtonStyle());
    auto btn_auth_quit = Button("Quit", on_quit, MakeButtonStyle());

    auto layout = Container::Vertical({
        btn_auth_register,
        btn_auth_login,
        btn_auth_quit,
    });

    auto renderer = Renderer(layout, [&, btn_auth_register, btn_auth_login, btn_auth_quit] {
        return vbox({
                   text("IM Client") | bold | hcenter,
                   separator(),
                   vbox({
                       btn_auth_register->Render(),
                       separator(),
                       btn_auth_login->Render(),
                       separator(),
                       btn_auth_quit->Render(),
                   }) | size(WIDTH, EQUAL, 30) |
                       border | center,
               }) |
               center;
    });

    return {layout, renderer};
}
