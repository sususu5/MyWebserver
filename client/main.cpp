#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>

int main() {
    using namespace ftxui;

    auto screen = ScreenInteractive::TerminalOutput();
    auto button = Button("Click me", [] { std::cout << "Clicked!\n"; });

    screen.Loop(button);
}