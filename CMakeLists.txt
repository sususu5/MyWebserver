cmake_minimum_required(VERSION 3.21)

# Top-level project declaration
project(TermChat)

# ---------------------------------------------------------------------------
# Global build options
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # For libraries that might be linked statically or shared

# Export compile_commands.json for IDEs (VSCode, clangd, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optionally select build type from command line, default to Release if none given
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

# ---------------------------------------------------------------------------
# vcpkg dependencies
# ---------------------------------------------------------------------------
find_package(protobuf CONFIG REQUIRED)
message(STATUS "Found Protobuf: ${protobuf_VERSION}")

# Optionally build only the client
option(BUILD_SERVER "Build the server application" ON)

# ---------------------------------------------------------------------------
# Rust toolchain (for cpp-rs-driver) - Only needed for server
# ---------------------------------------------------------------------------
if(BUILD_SERVER)
    find_program(CMAKE_Rust_COMPILER rustc REQUIRED)
    find_program(CARGO_EXECUTABLE cargo REQUIRED)

    message(STATUS "Using rustc: ${CMAKE_Rust_COMPILER}")
    message(STATUS "Using cargo: ${CARGO_EXECUTABLE}")

    # ---------------------------------------------------------------------------
    # ScyllaDB C++ driver (cpp-rs-driver)
    # ---------------------------------------------------------------------------
    include(FetchContent)

    set(CASS_BUILD_STATIC OFF CACHE BOOL "Disable static build for cpp-rs-driver" FORCE)
    set(CASS_BUILD_SHARED ON CACHE BOOL "Enable shared build for cpp-rs-driver" FORCE)
    set(CASS_BUILD_EXAMPLES OFF CACHE BOOL "Disable cpp-rs-driver examples" FORCE)
    set(CASS_BUILD_INTEGRATION_TESTS OFF CACHE BOOL "Disable cpp-rs-driver tests" FORCE)

    FetchContent_Declare(
        cpp_rs_driver
        GIT_REPOSITORY https://github.com/scylladb/cpp-rs-driver.git
        GIT_TAG v0.6.0
    )
    FetchContent_MakeAvailable(cpp_rs_driver)
endif()

# ---------------------------------------------------------------------------
# Sub-directories
# ---------------------------------------------------------------------------
add_subdirectory(proto)

if(BUILD_SERVER)
    add_subdirectory(server/src)
endif()

add_subdirectory(client)

# ---------------------------------------------------------------------------
# Packaging / installation rules (optional)
# ---------------------------------------------------------------------------
# install(TARGETS server RUNTIME DESTINATION bin)
