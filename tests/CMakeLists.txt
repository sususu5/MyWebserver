# CMake build script for tests inside the "test" directory

# Only one test target currently: test_runner
add_executable(test_runner test.cpp)

# Link against the core project library; this brings in pthread and mysqlclient
# transitively because termchat_core PUBLIC-links them.
target_link_libraries(test_runner PRIVATE termchat_core)

# If more tests are added, list them here using add_executable / add_test etc.
