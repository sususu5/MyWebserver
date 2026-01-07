# ---------------------------------------------------------------------------
# Collect source files
# ---------------------------------------------------------------------------
file(GLOB_RECURSE PROJECT_SRC CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/buffer/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/http/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/log/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/pool/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/server/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/timer/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/dao/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/service/*.cpp"
)

# Remove main.cpp from the library source list
list(REMOVE_ITEM PROJECT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

# ---------------------------------------------------------------------------
# Define static library
# ---------------------------------------------------------------------------
add_library(mywebserver STATIC ${PROJECT_SRC})

# Public include path
target_include_directories(mywebserver PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---------------------------------------------------------------------------
# Third-party / system dependencies
# ---------------------------------------------------------------------------
find_package(Threads REQUIRED)

find_package(sqlpp11 CONFIG REQUIRED COMPONENTS MariaDB)

find_package(Protobuf REQUIRED)

# ---------------------------------------------------------------------------
# Link all dependencies
# ---------------------------------------------------------------------------
target_link_libraries(mywebserver PUBLIC 
    Threads::Threads 
    protobuf::libprotobuf
    sqlpp11::sqlpp11
    sqlpp11::mariadb
)

if(TARGET proto_generated)
    target_link_libraries(mywebserver PUBLIC proto_generated)
endif()

# ---------------------------------------------------------------------------
# Server executable target
# ---------------------------------------------------------------------------
add_executable(server "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

target_link_libraries(server PRIVATE mywebserver)
