#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>

using namespace ftxui;

// Make button style for all buttons
ButtonOption MakeButtonStyle();
// Make password input option for all password inputs
InputOption MakePasswordOption();

// Helper to create a modal layer
Component Modal(Component main, Component modal, bool* show_modal);
// Helper to create an error message modal
Component ErrorModal(const std::string& error_msg, std::function<void()> on_close);
