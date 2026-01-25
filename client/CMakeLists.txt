find_package(ftxui CONFIG REQUIRED)

add_executable(client main.cpp)
target_sources(client PRIVATE
    auth/auth_page.cpp
    auth/login_page.cpp
    auth/register_page.cpp
    home_page.cpp
    ui_common.cpp
)

target_link_libraries(client PUBLIC
    ftxui::screen
    ftxui::dom
    ftxui::component
)