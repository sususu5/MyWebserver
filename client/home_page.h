#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <functional>

struct HomePage {
    ftxui::Component layout;
    ftxui::Component renderer;
};

HomePage BuildHomePage(const std::function<void()>& on_logout);
