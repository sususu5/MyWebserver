# TermChat

## 📖 Introduction
**TermChat** is a robust, CLI-based Instant Messaging system built with modern C++20. It leverages industry-proven technologies like the Reactor pattern (Epoll), Protocol Buffers for efficient serialization, and ScyllaDB for high-throughput data persistence. The project features a complete backend server and a rich TUI (Text User Interface) client built with FTXUI.

## 🛠 Tech Stack
- **Language**: C++20
- **Network Model**: Linux Epoll (Reactor Pattern) / Non-blocking I/O
- **Protocol**: Google Protobuf 3 (Binary Serialization)
- **Database**:
    - **ScyllaDB**: High-performance NoSQL for message storage (Current)
    - **MySQL**: Relational data (User auth/Friends) - *Legacy/Transition*
- **Client UI**: FTXUI (Functional Terminal User Interface)
- **Concurrency**: Thread Pool & Connection Pools
- **Build System**: CMake (Presets) + Vcpkg
- **DevOps**: Docker & DevContainer support

---

## 📂 Directory Structure

```text
/
├── client/              # FTXUI-based Terminal Client
│   ├── ui/              # UI Components (Auth, Chat, Friend panels)
│   ├── network_manager* # Client-side networking & state management
│   └── main.cpp         # Client entry point
├── server/
│   └── src/             # Core Backend Logic
│       ├── main.cpp     # Entry point
│       ├── core/        # Reactor core (Epoll, Webserver)
│       ├── service/     # Business Logic (Auth, Msg, Friend, Push)
│       ├── dao/         # Data Access Objects (ScyllaDB/MySQL)
│       ├── handler/     # Protocol Dispatchers (HTTP/Protobuf)
│       ├── pool/        # Connection Pools (Thread, SQL, Scylla)
│       └── buffer/      # Zero-copy buffer management
├── proto/               # Protobuf definitions (.proto files)
├── tests/               # Python Functional Tests & C++ Unit Tests
├── .devcontainer/       # VS Code DevContainer config
├── vcpkg.json           # Dependency management
└── docker-compose.yml   # Multi-container orchestration
```

---

## ✨ Features

### ✅ Core Functionality
- **High-Performance Server**: Event-driven architecture handling concurrent connections.
- **Protocol Buffers**: Compact and efficient binary message format.
- **Cross-Platform Client**: TUI client works on macOS, Linux, and Windows (via WSL/Docker).

### 🔐 Authentication & Security
- [x] **User Registration & Login**: Secure credential handling.
- [x] **JWT Authentication**: Stateless session management using JSON Web Tokens.
- [x] **Connection Management**: Heartbeats and automatic timeouts.

### 💬 Messaging
- [x] **P2P Messaging**: Real-time private messaging between users.
- [x] **Offline Messages**: Messages sent to offline users are stored (ScyllaDB) and pushed upon reconnection.
- [x] **Message History**: Persistent chat history retrieval.
- [x] **Rich TUI**: Scrollable chat history, real-time updates, and visual status indicators.

### 👥 Social Graph
- [x] **Friend System**: Send, Accept, and Reject friend requests.
- [x] **Real-time Notifications**: Instant push notifications for friend requests and status updates.
- [x] **Contact List**: Dynamic friend list with online status (partial).

---

## 🗺️ Roadmap

- [ ] **Group Chat**: Implementation of multi-user chat rooms.
- [ ] **File Transfer**: Support for sending images and files.
- [ ] **End-to-End Encryption**: Integrate Signal Protocol or similar for privacy.
- [ ] **Message Acknowledgement (ACK)**: Delivery receipts (Sent/Delivered/Read).
- [ ] **Search**: Full-text search for message history.
- [ ] **Metrics**: Prometheus/Grafana integration for server monitoring.

---

## 💻 Development Environment (Recommended)

This project is configured with a **DevContainer**. This is the preferred way to develop, as it provides a pre-configured environment with all dependencies (C++20, CMake, Vcpkg, MySQL, etc.).

1. Install [Docker Desktop](https://www.docker.com/products/docker-desktop).
2. Install [VS Code](https://code.visualstudio.com/) and the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers).
3. Open this folder in VS Code.
4. Click **"Reopen in Container"** when prompted (or use the command palette: `Dev Containers: Reopen in Container`).

### Build & Run (Inside DevContainer)

**Build:**
```bash
cmake --preset release
cmake --build build/release
```

```bash
cmake --preset debug
cmake --build build/debug
```

Every time the sql files are changed, the project needs to be re-compiled to generate the C++ models.

**Run Server & Test:**
```bash
./build/debug/server/src/server
./build/release/server/src/server

# Test Auth
python3 tests/test_auth.py [username] [password]
# Test Friend System (including Push)
python3 tests/test_friend.py
# Test P2P Message System
python3 tests/test_message.py

# Run Client (FTXUI)
./build/debug/client/client
./build/release/client/client

# If you want to use db visualization with scylla, run the following commands in devcontainer terminal
apt-get update
apt install openjdk-21-jdk
```

---

## 🚀 Quick Start (Docker Compose)

If you just want to run the server without setting up a development environment:

```bash
docker-compose up --build
```

---

## 🖥️ Local Client Development (macOS/Linux)

If you want to run the **FTXUI Client** locally on your host machine while the backend runs in Docker, follow these steps. This setup isolates the server environment while giving you a native terminal UI experience.

### 1. Start the Backend (Docker)

Start the Server, MySQL, and ScyllaDB in the background. We mount the generated certificates so the server can connect securely.

```bash
# Build and start services in detached mode
docker compose up --build -d
```

### 2. Build the Client (Local)

We use a special CMake preset (`macos-debug`) that:
*   **Skips server dependencies** (MySQL, ScyllaDB drivers are NOT required locally).
*   **Only builds the client** and protocol buffers.
*   **Uses Ninja** for faster builds.

```bash
# 1. Configure the project (Client Only)
cmake --preset macos-debug

# 2. Build the client executable
cmake --build --preset macos-debug --target client
```

### 3. Run the Client

```bash
./build/macos-debug/client/client
```

### ❓ Troubleshooting

*   **Database Internal Error / TLS Error**:
    If the client says "Database internal error", check the server logs:
    ```bash
    docker compose logs server
    ```
    If you see `TLS/SSL error: No such file or directory`, ensure you ran `docker compose down -v` to reset the certificate volumes and that `docker-compose.yml` mounts `ssl-certs:/etc/mysql/certs:ro` for the server.

*   **CMake Error: Generator Ninja**:
    If you see an error about "Unix Makefiles" vs "Ninja", run `rm -rf build/macos-debug` to clear the cache.

---

**Code Formatting:**
Using Google Style for C++: PascalCase for class names and function names, snake_case for variable names.

```bash
find server/src tests -name "*.h" -o -name "*.cpp" | xargs clang-format -i
```

*Note: The server currently listens on port 1316.*
