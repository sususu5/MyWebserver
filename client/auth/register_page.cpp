#include "register_page.h"
#include <ftxui/dom/elements.hpp>
#include "../ui_common.h"

using namespace ftxui;

RegisterPage BuildRegisterPage(std::string& username, std::string& password, const std::function<void()>& on_submit,
                               const std::function<void()>& on_back) {
    Component input_reg_user = Input(&username, "Username");
    Component input_reg_pass = Input(&password, "Password", MakePasswordOption());
    auto btn_reg_submit = Button("Submit", on_submit, MakeButtonStyle());
    auto btn_reg_back = Button("Back", on_back, MakeButtonStyle());

    auto layout = Container::Vertical({
        input_reg_user,
        input_reg_pass,
        btn_reg_submit,
        btn_reg_back,
    });

    auto renderer = Renderer(layout, [&, input_reg_user, input_reg_pass, btn_reg_submit, btn_reg_back] {
        return vbox({
                   text("Register") | bold | hcenter,
                   separator(),
                   vbox({
                       hbox(text("Username: "), input_reg_user->Render()),
                       hbox(text("Password: "), input_reg_pass->Render()),
                       separator(),
                       hbox(btn_reg_submit->Render() | flex, btn_reg_back->Render() | flex),
                   }) | size(WIDTH, GREATER_THAN, 40) |
                       border | center,
               }) |
               center;
    });

    return {layout, renderer};
}
