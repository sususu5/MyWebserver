# MyWebserver (Evolution to CLI IM)

## 📖 Introduction
This project is evolving from a high-performance C++ WebServer into a **CLI Instant Messaging (IM) System**. 
The goal is to build a robust, scalable backend using modern C++ standards (C++20) and industry-proven technologies.

## 🛠 Tech Stack
- **Language**: C++20
- **Network Model**: Linux Epoll (Reactor Pattern)
- **Protocol**: Google Protobuf (Binary Protocol)
- **Database**: MySQL
- **Logging**: Spdlog (Structured, Asynchronous)
- **Build System**: CMake + Vcpkg
- **Containerization**: Docker & Docker Compose

---

## 🗺️ Development Roadmap

### Phase 1: Protocol & Core Modernization (Current Focus)
**Goal**: Replace legacy HTTP parsing with efficient binary protocols and modernize the codebase.

- [ ] **1.1 C++20 Standard Migration**
  - [ ] Verify `CMakePresets.json` is set to C++20.
  - [ ] Refactor legacy C++98/11 patterns (use `std::span`, `std::format` if available, `auto`, `concepts`).
  - [ ] Replace raw pointers with smart pointers (`std::unique_ptr`, `std::shared_ptr`) where applicable.

- [ ] **1.2 Protobuf Protocol Implementation**
  - [ ] Define `message.proto` schemas (Login, Chat, Ack, Heartbeat).
  - [ ] Implement `Packet` codec (Length-Prefix framing) to handle TCP sticky/split packets.
  - [ ] Replace `HttpRequest/Response` classes with a generic `MessageContext`.
  - [ ] Implement a `ProtobufDispatcher` to route messages by `CommandID`.

- [ ] **1.3 Connection Layer Refactoring**
  - [ ] Rename `HttpConn` to `ImSession`.
  - [ ] Remove all HTTP-specific state machine logic.
  - [ ] Add session state management (Authenticated, Connected, Disconnected).

### Phase 2: Observability & Performance
**Goal**: Introduce enterprise-grade logging and profiling tools.

- [ ] **2.1 Logging System Upgrade**
  - [ ] Replace custom singleton logger with **spdlog**.
  - [ ] Configure asynchronous logging pattern (non-blocking I/O).
  - [ ] Implement structured logging (JSON support) for future ELK integration.
  - [ ] Add distinct log levels (Trace, Debug, Info, Warn, Error).

- [ ] **2.2 Performance Profiling & Optimization**
  - [ ] Integrate **gprof** or **Linux perf** tools.
  - [ ] Conduct baseline benchmark (QPS/Latency) using the new Protobuf protocol.
  - [ ] Generate Flame Graphs to identify CPU hotspots.
  - [ ] Optimize lock contention in `ThreadPool` and `SqlConnPool`.

### Phase 3: Data Persistence & Architecture
**Goal**: Switch to a more reliable database and decouple architecture.

- [ ] **3.1 Database Migration (PostgreSQL)**
  - [ ] Switch from MySQL to PostgreSQL (better JSONB support for message history).
  - [ ] Integrate `libpqxx` (C++ client for PG).
  - [ ] Design new schema: `Users` (UUID), `Messages` (Time-partitioned), `Contacts`.

- [ ] **3.2 Architecture Layering**
  - [ ] **Network Layer**: Pure `Epoll` loop, agnostic of business logic.
  - [ ] **Service Layer**: Business logic (e.g., `UserService`, `ChatService`).
  - [ ] **DAO Layer**: Database access objects.
  - [ ] **CLI Client**: Develop a standalone C++ CLI client for end-to-end testing.

### Phase 4: Production Features (IM Specific)
- [ ] **4.1 Reliability**
  - [ ] Message ACK mechanism (Ensure delivery).
  - [ ] Offline message storage (Store & Forward).
  - [ ] Heartbeat & Keep-alive detection.

- [ ] **4.2 Security**
  - [ ] TLS/SSL support (OpenSSL).
  - [ ] Password hashing (Argon2 or BCrypt).

---

## 🚀 Quick Start (Docker)

**Build & Run:**
```bash
docker-compose up --build
```

**Code Formatting:**
```bash
find code test -name "*.h" -o -name "*.cpp" | xargs clang-format -i
```

*Note: The server currently listens on port 1316.*

## 📂 Directory Structure
```
.
├── src
│   ├── proto       # Protobuf definitions
│   ├── server      # Epoll Reactor & Session management
│   ├── service     # Business Logic (New)
│   ├── dao         # Data Access Objects (New)
│   ├── utils       # Logger (spdlog), Config
│   └── main.cpp
├── tests           # Unit Tests & Benchmarks
├── docker-compose.yml
└── README.md
```
