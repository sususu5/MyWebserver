find_package(ftxui CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

add_executable(client main.cpp)
target_sources(client PRIVATE
    auth/auth_page.cpp
    auth/login_page.cpp
    auth/register_page.cpp
    home_page.cpp
    ui_common.cpp
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