# TermChat (CLI IM System) Context

## Project Overview
**TermChat** (formerly MyWebserver) is a high-performance C++ application evolving into a **CLI Instant Messaging (IM) System**. It utilizes modern C++20 standards, a Reactor-based networking model, and Google Protobuf for efficient binary communication. The system features a server backend and a terminal-based client user interface.

## Key Technologies
*   **Language:** C++20
*   **Build System:** CMake (with Presets), Vcpkg (Dependency Management)
*   **Networking:** Linux Epoll (Reactor Pattern), Non-blocking I/O
*   **Protocol:** Google Protobuf
*   **Database:**
    *   **New:** ScyllaDB (via `cpp-rs-driver`) - *Current focus*
    *   **Legacy/Transition:** MySQL (via `sqlpp11`)
*   **Client UI:** FTXUI (Functional Terminal User Interface)
*   **Other Libs:** `jwt-cpp` (Authentication), `nlohmann-json`, `spdlog` (format supported in custom logger)
*   **Containerization:** Docker, DevContainer support

## Architecture
### Server (`server/src/`)
*   **Core:** Custom `Webserver` class utilizing `Epoller` for event handling.
*   **Concurrency:** Thread Pool for task execution and connection handling.
*   **Storage:** DAO layer abstracting database access (`msg_scylla_dao`, `friend_dao`, etc.).
*   **Service Layer:** Business logic separated into services (`AuthService`, `FriendService`, `MsgService`, `PushService`).
*   **Logging:** Custom asynchronous logging system.

### Client (`client/`)
*   **UI:** Terminal-based UI built with **FTXUI**.
*   **Structure:** Network manager handles communication; UI components (`home_page`, `auth_page`, etc.) manage display and user input.

### Protocol (`proto/`)
*   Defines message structures and service contracts using `.proto` files.
*   CMake automatically generates C++ code from these definitions.

## Directory Structure
*   `server/src/`: Core server source code.
*   `client/`: Client application source code.
*   `proto/`: Protobuf definition files.
*   `tests/`: Python functional tests and C++ unit tests.
*   `sql/`: Database schema definitions.
*   `vcpkg.json`: Project dependencies.
*   `docker-compose.yml`: Deployment configuration.

## Build & Run

**Prerequisites:**
The project is best developed using the provided **DevContainer** (requires Docker & VS Code). Alternatively, ensure `cmake`, `vcpkg`, `clang`, and necessary DB drivers are installed.

**Build Commands (inside DevContainer or configured env):**
```bash
# Configure (Release or Debug)
cmake --preset release
# OR
cmake --preset debug

# Build
cmake --build build/release
# OR
cmake --build build/debug
```

**Run Server:**
```bash
./build/debug/server/src/server
# OR
./build/release/server/src/server
# Note: Server listens on port 1316 by default.
```

**Run Client:**
```bash
./build/debug/client/client
# OR
./build/release/client/client
```

## Testing
The project includes Python scripts for functional testing of specific features.

```bash
# Test Authentication
python3 tests/test_auth.py [username] [password]

# Test Friend System
python3 tests/test_friend.py

# Test Messaging
python3 tests/test_message.py
```

## Performance Benchmarks
*   **Throughput:** ~16,000+ QPS (Query Per Second) for P2P messaging.
*   **Test Environment:** 500 concurrent clients, 2,000 messages each.
*   **Efficiency:** Achieved via Reactor-based event handling and asynchronous batch persistence to ScyllaDB.

## Development Status & Roadmap
*   **Core Achievements:**
    *   **ScyllaDB Integration:** Fully functional asynchronous message persistence layer.
    *   **High Concurrency:** Stable performance with thousands of concurrent requests.
    *   **User Auth:** JWT-based session management.
    *   **Real-time Push:** Instant message delivery to online users.

*   **Current Focus:**
    *   **Password Encryption:** Implementing bcrypt/Argon2 (currently plain text).
    *   **Message Delivery Guarantees:** Implementing Ack mechanisms and retry logic.
    *   **Offline Storage:** Enhancing sync logic for massive offline message retrieval.


## Conventions
*   **Code Style:** Google Style (PascalCase for classes/functions, snake_case for variables).
*   **Formatting:** `clang-format` used on `.h` and `.cpp` files.
