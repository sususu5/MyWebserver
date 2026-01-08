# MyWebserver (CLI IM Evolution)

## Project Overview
This project is a high-performance C++ WebServer currently being refactored into a **CLI Instant Messaging (IM) System**. Built with Modern C++ (C++20), it leverages the **Reactor Pattern** using **Linux Epoll** for efficient non-blocking I/O. 

**Key Architectural Components:**
*   **Networking:** Epoll-based event loop (`Epoller`, `Webserver`) handling concurrent connections.
*   **Concurrency:** Custom `ThreadPool` for processing tasks (reading, writing, logic) off the main event loop.
*   **Protocol:** **Google Protobuf** for binary message serialization (definitions in `proto/`).
*   **Database:** **MySQL/MariaDB** accessed via **sqlpp11** (Type-safe Embedded Domain Specific Language).
*   **Connection Pool:** `SqlConnPool` managing `sqlpp::mysql::connection` objects.
*   **Logging:** Structured, asynchronous logging system (`Log`).
*   **Build System:** CMake with Vcpkg for dependency management.

## Building and Running

### Prerequisites
*   **DevContainer (Recommended):** The project is configured for VS Code DevContainers, providing a pre-configured environment with C++20, CMake, Ninja, Vcpkg, and Python3 (for code generation).
*   **Manual Setup:** Requires C++20 compliant compiler, CMake (>3.21), Ninja, Python3, and Vcpkg.

### Commands

**1. Build (Release):**
```bash
cmake --preset release
cmake --build build/release
```

**2. Build (Debug):**
```bash
cmake --preset debug
cmake --build build/debug
```
*Note: The build process automatically runs `scripts/ddl2cpp` to generate C++ database models from `sql/schema.sql` into `server/src/model/`.*

**3. Run Server:**
```bash
./build/release/server/src/server
# or
./build/debug/server/src/server
```

**4. Run with Docker Compose:**
Useful for quick deployment or running outside the DevContainer.
```bash
docker-compose up --build
```

**5. Run Tests:**
```bash
cmake --build build/release --target test_runner
./build/release/test/test_runner
```

## Directory Structure

*   **`proto/`**: Contains `.proto` files defining the communication protocol (e.g., `message.proto`).
*   **`server/src/`**: The core source code.
    *   `main.cpp`: Entry point. Initializes and starts the `Webserver`.
    *   `server/`: Contains the `Webserver` class (manages the reactor loop) and `Epoller` wrapper.
    *   `model/`: **Auto-generated** C++ structs representing database tables (do not edit manually).
    *   `dao/`: Data Access Objects (e.g., `UserDao`) utilizing `sqlpp11` and `model/`.
    *   `pool/`: Thread pool and MySQL connection pool implementations.
    *   `log/`: Logging system implementation.
    *   `timer/`: Heap-based timer for managing connection timeouts.
    *   `buffer/`: Custom buffer implementation for I/O.
*   **`scripts/`**: Utility scripts, including `ddl2cpp` for generating C++ models from SQL.
*   **`sql/`**: Database schema definitions (`schema.sql`).
*   **`.devcontainer/`**: Configuration for the VS Code development container.

## Development Conventions

*   **Language Standard:** C++20.
*   **Build System:** CMake is the source of truth.
*   **Dependencies:** Managed via `vcpkg`.
*   **Database Access:** 
    *   Always use `SqlConnPool::Instance()` to get a `sqlpp::mysql::connection`.
    *   Use `UserDao` or create new DAOs for database interactions.
    *   **Do not** use raw SQL strings; utilize `sqlpp11`'s type-safe query builder.
    *   If you modify `sql/schema.sql`, rebuild the project to update the generated C++ models.
*   **Coding Style:** Google C++ Style Guide (enforced via `.clang-format`).
*   **Logging:** Use `LOG_*` macros (e.g., `LOG_INFO`, `LOG_ERROR`).
