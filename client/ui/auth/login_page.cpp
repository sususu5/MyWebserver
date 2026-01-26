#include "login_page.h"
#include <ftxui/dom/elements.hpp>
#include "../ui_common.h"

using namespace ftxui;

LoginPage BuildLoginPage(std::string& username, std::string& password, const std::function<void()>& on_submit,
                         const std::function<void()>& on_back) {
    Component input_login_user = Input(&username, "Username");
    Component input_login_pass = Input(&password, "Password", MakePasswordOption());
    auto btn_login_submit = Button("Login", on_submit, MakeButtonStyle());
    auto btn_login_back = Button("Back", on_back, MakeButtonStyle());

    auto layout = Container::Vertical({
        input_login_user,
        input_login_pass,
        btn_login_submit,
        btn_login_back,
    });

    auto renderer = Renderer(layout, [&, input_login_user, input_login_pass, btn_login_submit, btn_login_back] {
        return vbox({
                   text("Login") | bold | hcenter,
                   separator(),
                   vbox({
                       hbox(text("Username: "), input_login_user->Render()),
                       hbox(text("Password: "), input_login_pass->Render()),
                       separator(),
                       hbox(btn_login_submit->Render() | flex, btn_login_back->Render() | flex),
                   }) | size(WIDTH, GREATER_THAN, 40) |
                       border | center,
               }) |
               center;
    });

    return {layout, renderer};
}
