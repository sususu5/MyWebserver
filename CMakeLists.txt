cmake_minimum_required(VERSION 3.12)

# Top-level project declaration
project(MyWebserver LANGUAGES CXX)

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
# Sub-directories
# ---------------------------------------------------------------------------
add_subdirectory(code)
add_subdirectory(test)

# ---------------------------------------------------------------------------
# Packaging / installation rules (optional)
# ---------------------------------------------------------------------------
# install(TARGETS server RUNTIME DESTINATION bin)
