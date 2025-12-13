cmake_minimum_required(VERSION 3.21)

# Top-level project declaration
project(MyWebserver VERSION 1.0.0 LANGUAGES CXX)

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

# ---------------------------------------------------------------------------
# Protobuf code generation configuration
# ---------------------------------------------------------------------------
# Proto file directory
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

# Find all .proto files
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")

# Generate C++ code for each .proto file
set(PROTO_SRCS)
set(PROTO_HDRS)
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    list(APPEND PROTO_SRCS "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.cc")
    list(APPEND PROTO_HDRS "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.h")
    
    add_custom_command(
        OUTPUT "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.cc" "${PROTO_OUT_DIR}/${PROTO_NAME}.pb.h"
        COMMAND protobuf::protoc
        ARGS --cpp_out=${PROTO_OUT_DIR} -I${PROTO_DIR} ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating C++ code from ${PROTO_NAME}.proto"
    )
endforeach()

# Create proto generated library
if(PROTO_SRCS)
    add_library(proto_generated STATIC ${PROTO_SRCS} ${PROTO_HDRS})
    target_include_directories(proto_generated PUBLIC ${PROTO_OUT_DIR})
    target_link_libraries(proto_generated PUBLIC protobuf::libprotobuf)
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
