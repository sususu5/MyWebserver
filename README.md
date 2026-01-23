# MyWebserver (Evolution to CLI IM)

## 📖 Introduction
This project is evolving from a high-performance C++ WebServer into a **CLI Instant Messaging (IM) System**. 
The goal is to build a robust, scalable backend using modern C++ standards (C++20) and industry-proven technologies.

## 🛠 Tech Stack
- **Language**: C++20
- **Network Model**: Linux Epoll (Reactor Pattern)
- **Protocol**: Google Protobuf (Binary Serialization)
- **Database Layer**: sqlpp11 (Modern C++ EDSL for SQL - *In Transition*)
- **Concurrency**: Thread Pool & MySQL Connection Pool
- **Logging**: Custom Asynchronous Logging (support for Spdlog format)
- **Build System**: CMake (Presets) + Vcpkg
- **Toolchain**: clangd + compile_commands.json

---

## 📂 Directory Structure

```text
/
├── proto/               # Protocol definitions (.proto files)
├── server/
│   └── src/             # Core Backend Logic
│       ├── main.cpp     # Entry point
│       ├── buffer/      # Custom I/O buffer management
│       ├── dao/         # Data Access Objects (Database logic)
│       ├── service/     # Business logic layer (Auth, Chat, Friend, Push, etc.)
│       ├── core/        # Webserver & Epoller (Reactor core)
│       ├── pool/        # ThreadPool & SqlConnPool
│       ├── http/        # HTTP protocol handling (legacy)
│       ├── log/         # Async logging system
│       ├── timer/       # Heap-based timer for timeouts
│       ├── util/        # Utilities (UUID, Token, etc.)
│       └── CMakeLists.txt
├── resources/           # Static assets (HTML, JS, CSS)
├── test/                # Unit tests & Benchmarking
├── build/               # Build artifacts (generated pb files)
├── .devcontainer/       # VS Code DevContainer config
├── vcpkg.json           # Dependency management
├── CMakePresets.json    # Build presets configuration
└── docker-compose.yml   # Multi-container orchestration
```

---

## 🗺️ Development Roadmap

> After updating the proto files, the project needs to be re-compiled to generate the C++ models.

### Phase 1: Security & Foundation 🔐

- [ ] **Password Encryption**: Replace plaintext storage with bcrypt/Argon2 + salt
- [ ] **JWT Secret Management**: Move hardcoded secret to environment variable
- [ ] **Heartbeat Mechanism**: Implement client-server keepalive (PING/PONG)
- [ ] **Message ID Generator**: Implement Snowflake algorithm for distributed unique IDs
- [x] **User Online Status**: In-memory status map (Partially implemented via `PushService`)

### Phase 2: Core Messaging 💬

- [ ] **P2P Message Send/Receive**: Implement `CMD_P2P_MSG_REQ` / `CMD_P2P_MSG_PUSH`
- [ ] **Message Persistence**: Store messages in MySQL/MongoDB
- [ ] **Message ACK Confirmation**: Implement `MessageAck` logic for delivery guarantee
- [ ] **Offline Message Storage**: Create offline message table and pull mechanism
- [ ] **Message Retry**: Exponential backoff retry for failed deliveries

### Phase 3: Social Features 👥

- [x] **Friend Request Flow**: Request → Accept/Reject → Notification
- [x] **Friend List**: Retrieve friend list
- [ ] **Friend Management**: Remove friend
- [ ] **User Profile**: Status message
- [ ] **Block User**: Implement user blocking functionality

### Phase 4: Architecture Evolution 🏗️

- [ ] **Master-Slave Reactor**: Evolve from single Reactor to multi-Reactor pattern
- [ ] **Redis Integration**: Session management, user routing, online status
- [ ] **Message Queue**: Integrate Kafka/RabbitMQ for async processing
- [ ] **Multi-Node Deployment**: Stateless server + message routing layer
- [ ] **Load Balancing**: Consistent hashing for user-server mapping

### Phase 5: Production Readiness 🚀

- [ ] **Rate Limiting**: Token bucket / Sliding window algorithm
- [ ] **Circuit Breaker**: Graceful degradation under high load
- [ ] **Monitoring**: Prometheus metrics + Grafana dashboards
- [ ] **Distributed Tracing**: Jaeger/Zipkin integration
- [ ] **Stress Testing**: Benchmark with webbench / wrk / custom tools
- [ ] **Configuration Center**: etcd/consul for dynamic config

### Completed ✅

- [x] User Registration (Protobuf)
- [x] User Login with JWT Token
- [x] Protocol auto-detection (HTTP/Protobuf)
- [x] MySQL Connection Pool
- [x] Thread Pool
- [x] Async Logging System
- [x] Heap-based Timer for Connection Timeout
- [x] Real-time Push Notifications (PushService)
- [x] Friend System (Req/Resp/Push)

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
```

---

## 🚀 Quick Start (Docker Compose)

If you just want to run the server without setting up a development environment:

```bash
docker-compose up --build
```

**Code Formatting:**
Using snake_case for variable names and function names, and CamelCase for class names.

```bash
find server/src tests -name "*.h" -o -name "*.cpp" | xargs clang-format -i
```

*Note: The server currently listens on port 1316.*