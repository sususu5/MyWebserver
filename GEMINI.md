# MyWebserver Project Context

## Project Overview
This project is a high-performance C++ server evolving from a standard web server into a **CLI Instant Messaging (IM) System**. It uses a **Reactor Pattern** with **Linux Epoll** for non-blocking I/O and **Protocol Buffers** for efficient binary communication.

**Key Technologies:**
*   **Language:** C++20
*   **Networking:** Linux Epoll (Edge Triggered/Level Triggered), custom Reactor implementation.
*   **Serialization:** Google Protocol Buffers (v3).
*   **Database:** MySQL/MariaDB, accessed via **sqlpp11** (Type-safe Embedded Domain Specific Language).
*   **Concurrency:** Custom `ThreadPool` and `SqlConnPool`.
*   **Build System:** CMake (>3.21) with Vcpkg for dependencies.

## Architecture
The system is built around a central `Webserver` class that manages the event loop:

1.  **Event Loop:** `Epoller` monitors file descriptors (sockets).
2.  **Connection Handling:** `Webserver` accepts new connections and wraps them in `TcpConnection` objects.
3.  **I/O:** Raw bytes are read into `Buffer` objects.
4.  **Protocol Processing:** `TcpConnection` (or a handler) deserializes Protobuf messages (`Envelope`).
5.  **Business Logic:** Requests are routed to services (e.g., `AuthService`) or handled directly.
6.  **Data Access:** Services use DAOs (e.g., `UserDao`) which claim connections from `SqlConnPool` to execute type-safe queries using `sqlpp11`.

## Building and Running

### Prerequisites
*   **DevContainer (Highly Recommended):** The project is configured for VS Code DevContainers, providing a consistent environment with all dependencies (C++20, CMake, Ninja, Vcpkg, Python3, MariaDB).
*   **Manual Setup:** Requires C++20 compiler, CMake, Ninja, Python3 (for `ddl2cpp`), Vcpkg, and development headers for MySQL/MariaDB.

### Build Commands (CMake Presets)
The project uses CMake presets for simplified configuration.

**1. Configure:**
```bash
cmake --preset release
# OR
cmake --preset debug
```

**2. Build:**
```bash
cmake --build build/release
# OR
cmake --build build/debug
```
*Note: The build process automatically runs `scripts/ddl2cpp` to generate C++ database models from `sql/schema.sql` into `server/src/model/`.*

### Running the Server
```bash
./build/release/server/src/server
```
The server listens on **port 1316** by default.

### Running with Docker Compose
For a quick startup without a dev environment:
```bash
docker-compose up --build
```

### Running Tests
```bash
cmake --build build/release --target test_runner
./build/release/test/test_runner
```

## Directory Structure

*   **`proto/`**: Protocol Buffer definitions (`.proto`).
    *   `message.proto`: Main envelope and messaging structures.
    *   `auth_service.proto`: Authentication RPC definitions.
*   **`server/src/`**: Source code.
    *   `main.cpp`: Entry point.
    *   `server/`: Core networking (`Webserver`, `Epoller`, `TcpConnection`).
    *   `model/`: **Auto-generated** C++ structs for DB tables (do not edit).
    *   `dao/`: Data Access Objects (`UserDao`).
    *   `pool/`: `ThreadPool` and `SqlConnPool`.
    *   `service/`: Business logic (`AuthService`).
    *   `log/`: Asynchronous logging.
*   **`sql/`**: Database schema (`schema.sql`).
*   **`scripts/`**: Utility scripts (`ddl2cpp` for model generation).
*   **`CMakeLists.txt`**: Main build configuration.
*   **`vcpkg.json`**: Dependency manifest.

## Development Conventions

*   **Database Models:** Do not manually edit files in `server/src/model/`. Modify `sql/schema.sql` instead and rebuild. The build system will regenerate the C++ code.
*   **Database Access:** Always use `SqlConnPool` to get a connection. Use `sqlpp11` for queries (no raw SQL strings).
*   **Logging:** Use the asynchronous `LOG_*` macros (e.g., `LOG_INFO`, `LOG_ERROR`).
*   **Style:** Follow the Google C++ Style Guide (enforced by `.clang-format`).
*   **Protocol:** All client-server communication should be defined in `proto/` and wrapped in the `Envelope` message type.
