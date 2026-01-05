# CMake build script for project sources found under the "code" directory

# ---------------------------------------------------------------------------
# Collect source files
# ---------------------------------------------------------------------------
# Gather all .cpp files in sub-directories *except* main.cpp (which will be
# compiled only into the final executable).
file(GLOB_RECURSE PROJECT_SRC CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Remove main.cpp from the library source list
list(REMOVE_ITEM PROJECT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

# ---------------------------------------------------------------------------
# Define static library
# ---------------------------------------------------------------------------
add_library(mywebserver STATIC ${PROJECT_SRC})

# Public include path so external targets can do #include "buffer/buffer.h" etc.
target_include_directories(mywebserver PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---------------------------------------------------------------------------
# Third-party / system dependencies
# ---------------------------------------------------------------------------
find_package(Threads REQUIRED)

# MySQL client library (System lib via apt-get)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# ---------------------------------------------------------------------------
# Link all dependencies
# ---------------------------------------------------------------------------
target_link_libraries(mywebserver PUBLIC 
    Threads::Threads 
    ${MYSQL_LIBRARIES}
    protobuf::libprotobuf
)
target_include_directories(mywebserver PUBLIC ${MYSQL_INCLUDE_DIRS})

if(TARGET proto_generated)
    target_link_libraries(mywebserver PUBLIC proto_generated)
endif()

# ---------------------------------------------------------------------------
# Server executable target
# ---------------------------------------------------------------------------
add_executable(server "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

target_link_libraries(server PRIVATE mywebserver)

# Allow `cmake --install` to deploy the binary (optional)
# install(TARGETS server RUNTIME DESTINATION bin)
