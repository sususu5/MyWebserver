#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <functional>

struct AuthPage {
    ftxui::Component layout;
    ftxui::Component renderer;
};

AuthPage BuildAuthPage(const std::function<void()>& on_register, const std::function<void()>& on_login,
                       const std::function<void()>& on_quit);