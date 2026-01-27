#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include "network_manager.h"
#include "ui/auth/auth_page.h"
#include "ui/auth/login_page.h"
#include "ui/auth/register_page.h"
#include "ui/home_page.h"
#include "ui/ui_common.h"

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
                error_msg_ = "Connection failed: Could not connect to server.";
                show_error_ = true;
                return;
            }
            if (NetworkManager::GetInstance().Register(reg_username_, reg_password_, error_msg_)) {
                login_username_ = reg_username_;
                login_password_ = reg_password_;
                current_page_ = (int)Page::LOGIN;
            } else {
                show_error_ = true;
            }
        };
        auto on_reg_back = [&] { current_page_ = (int)Page::AUTH; };
        auto register_page = BuildRegisterPage(reg_username_, reg_password_, on_reg_submit, on_reg_back);

        // Build login page
        auto on_login_submit = [&] {
            if (!NetworkManager::GetInstance().Connect(kServerHost, kServerPort)) {
                error_msg_ = "Connection failed: Could not connect to server.";
                show_error_ = true;
                return;
            }
            if (NetworkManager::GetInstance().Login(login_username_, login_password_, error_msg_)) {
                current_page_ = (int)Page::MAIN;
            } else {
                show_error_ = true;
            }
        };
        auto on_login_back = [&] { current_page_ = (int)Page::AUTH; };
        auto login_page = BuildLoginPage(login_username_, login_password_, on_login_submit, on_login_back);

        // Build home page
        auto on_logout = [&] {
            std::string temp_error;
            NetworkManager::GetInstance().Logout(temp_error);
            current_page_ = (int)Page::AUTH;
        };
        auto home_page = BuildHomePage(on_logout, &home_page_state_);

        // Build top level container
        auto tab_container = Container::Tab(
            {
                auth_page.renderer,
                register_page.renderer,
                login_page.renderer,
                home_page.renderer,
            },
            &current_page_);

        auto error_modal = ErrorModal(error_msg_, [&] { show_error_ = false; });
        main_container_ = Modal(tab_container, error_modal, &show_error_);
    }

    int current_page_ = (int)Page::AUTH;
    ScreenInteractive screen_ = ScreenInteractive::Fullscreen();
    Component main_container_;

    std::string reg_username_, reg_password_;
    std::string login_username_, login_password_;

    HomePageState home_page_state_;

    std::string error_msg_;
    bool show_error_ = false;
};

int main() {
    IMClient app;
    app.Run();
    return 0;
}