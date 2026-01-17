# MyWebserver Project Context

## Project Overview
This project is a high-performance C++ server designed as a **CLI Instant Messaging (IM) System Backend**, evolving from a standard web server. It utilizes a **Reactor Pattern** with **Linux Epoll** for non-blocking I/O and **Protocol Buffers** for efficient binary communication.

**Key Features:**
*   **Dual Protocol Support:** Handles both HTTP (static resources) and custom Protobuf-based IM protocol on the same port.
*   **High Concurrency:** Implements a Reactor model with Epoll (ET/LT modes) and a custom `ThreadPool`.
*   **Database Integration:** Uses **MySQL/MariaDB** accessed via **sqlpp11** (Type-safe Embedded Domain Specific Language) and a custom connection pool (`SqlConnPool`).
*   **Authentication:** **JWT (JSON Web Token)** based authentication for the IM protocol.

## Tech Stack
*   **Language:** C++20
*   **Build System:** CMake (>3.21) with Vcpkg for dependency management.
*   **Networking:** Linux Epoll, non-blocking I/O.
*   **Serialization:** Google Protocol Buffers (v3).
*   **Security:** `jwt-cpp` (with `nlohmann-json`) for token generation/validation; OpenSSL.
*   **Database:** MySQL/MariaDB, `sqlpp11`.

## Architecture

The system is built around a central `Webserver` class that manages the event loop:

1.  **Event Loop:** `Epoller` monitors file descriptors.
2.  **Connection:** `TcpConnection` wraps client sockets.
3.  **Protocol Dispatch:**
    *   **HTTP:** Handled by `HttpHandler` (serves static files from `resources/` and simple POST logic).
    *   **Protobuf:** Handled by `ProtobufHandler`. Dispatches commands (`CMD_REGISTER_REQ`, `CMD_LOGIN_REQ`, etc.) to `AuthService`.
4.  **Service Layer:** `AuthService` manages business logic (Registration, Login).
5.  **Data Access:** `UserDao` performs DB operations using connections from `SqlConnPool`.

## Build and Run

### Prerequisites
*   **Environment:** Linux/macOS (DevContainer recommended).
*   **Tools:** CMake, Ninja/Make, Python3 (for `ddl2cpp`), Vcpkg.

### Build Commands
The project uses CMake Presets.

**1. Configure:**
```bash
cmake --preset debug
# OR
cmake --preset release
```

**2. Build:**
```bash
cmake --build build/debug --target server
# OR
cmake --build build/release --target server
```
*Note: The build process automatically generates C++ DB models from `sql/schema.sql`.*

### Running the Server
```bash
./build/debug/server/src/server
```
*   **Port:** 1316 (default)
*   **DB:** Connects to `mysql` host (defined in `docker-compose.yml` or env).

### Running Tests
The project includes Python-based integration tests.
```bash
# Verify Auth (Register/Login)
python3 tests/test_auth.py
```

## Directory Structure
*   **`proto/`**: Protobuf definitions (`message.proto`, `auth_service.proto`).
*   **`server/src/`**: Core source code.
    *   **`core/`**: `Webserver`, `Epoller`, `TcpConnection`.
    *   **`handler/`**: `ProtobufHandler`, `HttpHandler`.
    *   **`service/`**: Business logic (`AuthService`).
    *   **`dao/`**: Data Access Objects (`UserDao`).
    *   **`utils/`**: Utilities like `TokenUtil` (JWT) and `UuidGenerator`.
    *   **`model/`**: **Auto-generated** SQLPP11 models.
*   **`sql/`**: Database schema (`schema.sql`).
*   **`tests/`**: Integration test scripts.

## Conventions
*   **Database Models:** Do NOT edit `server/src/model/`. Edit `sql/schema.sql` and rebuild.
*   **JWT Secrets:** Hardcoded in `TokenUtil` (for now) or loaded from env.
*   **Protocol:** Add new commands to `proto/message.proto` and handle them in `ProtobufHandler`.
