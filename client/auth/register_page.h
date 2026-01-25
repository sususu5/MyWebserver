#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <functional>

struct RegisterPage {
    ftxui::Component layout;
    ftxui::Component renderer;
};

RegisterPage BuildRegisterPage(std::string& username, std::string& password, const std::function<void()>& on_submit,
                               const std::function<void()>& on_back);