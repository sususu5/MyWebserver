# MyWebserver (CLI IM Evolution)

## Project Overview
This project is a high-performance C++ WebServer currently being refactored into a **CLI Instant Messaging (IM) System**. Built with Modern C++ (C++20), it leverages the **Reactor Pattern** using **Linux Epoll** for efficient non-blocking I/O. 

**Key Architectural Components:**
*   **Networking:** Epoll-based event loop (`Epoller`, `Webserver`) handling concurrent connections.
*   **Concurrency:** Custom `ThreadPool` for processing tasks (reading, writing, logic) off the main event loop.
*   **Protocol:** Moving towards **Google Protobuf** for binary message serialization (definitions in `proto/`).
*   **Database:** **MySQL** with a custom connection pool (`SqlConnPool`) for efficient resource management.
*   **Logging:** Structured, asynchronous logging system (`Log`, `Spdlog`).
*   **Build System:** CMake with Vcpkg for dependency management.

## Building and Running

### Prerequisites
*   **DevContainer (Recommended):** The project is configured for VS Code DevContainers, which provides a pre-configured environment with C++20, CMake, Ninja, Vcpkg, and MySQL client libraries.
*   **Manual Setup:** Requires C++20 compliant compiler, CMake (>3.21), Ninja, and Vcpkg.

### Commands

**1. Build (Release):**
```bash
cmake --preset release
cmake --build build/release
```

**2. Run Server:**
```bash
./build/release/server/src/server
```

**3. Run with Docker Compose:**
Useful for quick deployment or running outside the DevContainer.
```bash
docker-compose up --build
```
*Note: Ensure the executable path in `docker-compose.yml` matches the build output.*

**4. Run Tests:**
```bash
# Build tests
cmake --build build/release --target test_main
# Run
./build/release/test/test_main
```

## Directory Structure

*   **`proto/`**: Contains `.proto` files defining the communication protocol (e.g., `auth_service.proto`, `message.proto`) and its CMake configuration.
*   **`server/src/`**: The core source code.
    *   `main.cpp`: Entry point. Initializes and starts the `Webserver`.
    *   `server/`: Contains the `Webserver` class (manages the reactor loop) and `Epoller` wrapper.
    *   `http/`: HTTP parsing and response logic (legacy, likely to be replaced/augmented by Protobuf handlers).
    *   `pool/`: Thread pool and MySQL connection pool implementations.
    *   `log/`: Logging system implementation.
    *   `timer/`: Heap-based timer for managing connection timeouts.
    *   `buffer/`: Custom buffer implementation for I/O.
*   **`server/log/`**: Directory where runtime logs are stored.
*   **`resources/`**: Static assets (HTML, CSS, JS) - legacy from the WebServer phase.
*   **`test/`**: Unit tests and benchmarking tools (`webbench`).
*   **`.devcontainer/`**: Configuration for the VS Code development container.

## Development Conventions

*   **Language Standard:** C++20.
*   **Build System:** CMake is the source of truth. Always use `cmake` commands or the VS Code CMake extension.
*   **Dependencies:** Managed via `vcpkg`.
*   **Coding Style:**
    *   Google C++ Style Guide (inferred from `.clang-format` presence).
    *   Use `clang-format` to maintain consistency: `find server/src test -name "*.h" -o -name "*.cpp" | xargs clang-format -i`.
*   **Architecture Pattern:** Reactor pattern handling IO events, dispatching heavy lifting to a Thread Pool.
*   **Database Access:** Always use `SqlConnPool::Instance()` to get a connection. Prefer RAII wrappers (`SqlConnRAII`) to ensure connections are returned to the pool.
*   **Logging:** Use the `LOG_*` macros (e.g., `LOG_INFO`, `LOG_ERROR`) for logging.
