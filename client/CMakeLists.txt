find_package(ftxui CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

add_executable(client main.cpp)
target_sources(client PRIVATE
    ui/auth/auth_page.cpp
    ui/auth/login_page.cpp
    ui/auth/register_page.cpp
    ui/chat/chat_panel.cpp
    ui/friend/add_friend_panel.cpp
    ui/friend/handle_friend_panel.cpp
    ui/home_page.cpp
    ui/ui_common.cpp
    network_manager.cpp
)

if(TARGET proto_generated)
    add_dependencies(client proto_generated)
    target_link_libraries(client PUBLIC proto_generated)
endif()

target_link_libraries(client PUBLIC
    ftxui::screen
    ftxui::dom
    ftxui::component
    protobuf::libprotobuf
)