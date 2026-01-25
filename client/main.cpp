#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

using namespace ftxui;

// Page enum class
enum class Page { AUTH, REGISTER, LOGIN, MAIN };

class IMClient {
public:
    IMClient() { InitUI(); }

    void Run() { screen_.Loop(main_container_); }

private:
    void InitUI() {
        auto button_option = ButtonOption::Animated();
        button_option.transform = [](const EntryState& state) {
            auto content = text(state.label) | center;
            auto bg = state.focused ? Color::GrayLight : Color::GrayDark;
            return content | flex | bgcolor(bg);
        };

        // Auth page component
        auto register_button = Button("Register", [&] { current_page_ = (int)Page::MAIN; }, button_option);
        auto login_button = Button("Login", [&] { current_page_ = (int)Page::MAIN; }, button_option);
        auto quit_button = Button("Quit", screen_.ExitLoopClosure(), button_option);
        auto auth_component = Container::Vertical({
            register_button,
            login_button,
            quit_button,
        });

        auto auth_renderer = Renderer(auth_component, [&, register_button, login_button, quit_button] {
            auto menu_box = vbox({
                                register_button->Render() | flex | size(HEIGHT, EQUAL, 3),
                                separator(),
                                login_button->Render() | flex | size(HEIGHT, EQUAL, 3),
                                separator(),
                                quit_button->Render() | flex | size(HEIGHT, EQUAL, 3),
                            }) |
                            flex;

            return vbox({
                       text("IM Client") | bold | hcenter,
                       separator(),
                       menu_box | size(WIDTH, EQUAL, 20) | size(HEIGHT, EQUAL, 11) | border,
                   }) |
                   center;
        });

        // Main page component
        auto logout_button = Button(" Logout ", [&] { current_page_ = (int)Page::LOGIN; });
        auto main_component = Container::Vertical({
            logout_button,
        });

        auto main_renderer = Renderer(main_component, [&, logout_button] {
            return vbox({
                hbox({
                    text("Main Page") | bold,
                    filler(),
                    logout_button->Render(),
                }) | border,
                text("Welcome to IM System!") | center | flex,
            });
        });

        // Top level container (Tab switch)
        main_container_ = Container::Tab(
            {
                auth_renderer,  // LOGIN
                main_renderer,  // MAIN
            },
            &current_page_);
    }

    int current_page_ = (int)Page::AUTH;
    ScreenInteractive screen_ = ScreenInteractive::Fullscreen();
    Component main_container_;
};

int main() {
    IMClient app;
    app.Run();
    return 0;
}
