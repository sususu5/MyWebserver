#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <functional>

struct LoginPage {
    ftxui::Component layout;
    ftxui::Component renderer;
};

LoginPage BuildLoginPage(std::string& username, std::string& password, const std::function<void()>& on_submit,
                         const std::function<void()>& on_back);
