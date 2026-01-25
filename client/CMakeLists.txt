find_package(ftxui CONFIG REQUIRED)

add_executable(client main.cpp)

target_link_libraries(client PUBLIC
    ftxui::screen
    ftxui::dom
    ftxui::component
)