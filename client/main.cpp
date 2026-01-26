#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include "auth/auth_page.h"
#include "auth/login_page.h"
#include "auth/register_page.h"
#include "home_page.h"
#include "network_manager.h"

using namespace ftxui;

// Page enum class
enum class Page { AUTH, REGISTER, LOGIN, MAIN };

namespace {
constexpr const char* kServerHost = "127.0.0.1";
constexpr int kServerPort = 1316;
}  // namespace

class IMClient {
public:
    IMClient() { InitUI(); }

    void Run() { screen_.Loop(main_container_); }

private:
    void InitUI() {
        // Build auth page
        auto on_register = [&] { current_page_ = (int)Page::REGISTER; };
        auto on_login = [&] { current_page_ = (int)Page::LOGIN; };
        auto on_quit = screen_.ExitLoopClosure();
        auto auth_page = BuildAuthPage(on_register, on_login, on_quit);

        // Build register page
        auto on_reg_submit = [&] {
            if (!NetworkManager::GetInstance().Connect(kServerHost, kServerPort)) {
                return;
            }
            std::string error_msg;
            if (NetworkManager::GetInstance().Register(reg_username_, reg_password_, error_msg)) {
                current_page_ = (int)Page::MAIN;
            } else {
                // TODO: Show error message
            }
        };
        auto on_reg_back = [&] { current_page_ = (int)Page::AUTH; };
        auto register_page = BuildRegisterPage(reg_username_, reg_password_, on_reg_submit, on_reg_back);

        // Build login page
        auto on_login_submit = [&] { current_page_ = (int)Page::MAIN; };
        auto on_login_back = [&] { current_page_ = (int)Page::AUTH; };
        auto login_page = BuildLoginPage(login_username_, login_password_, on_login_submit, on_login_back);

        // Build home page
        auto on_logout = [&] { current_page_ = (int)Page::AUTH; };
        auto home_page = BuildHomePage(on_logout);

        // Build top level container
        main_container_ = Container::Tab(
            {
                auth_page.renderer,
                register_page.renderer,
                login_page.renderer,
                home_page.renderer,
            },
            &current_page_);
    }

    int current_page_ = (int)Page::AUTH;
    ScreenInteractive screen_ = ScreenInteractive::Fullscreen();
    Component main_container_;

    std::string reg_username_, reg_password_;
    std::string login_username_, login_password_;
};

int main() {
    IMClient app;
    app.Run();
    return 0;
}
