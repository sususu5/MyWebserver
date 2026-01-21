# MyWebserver (C++ IM Backend)

## 🔭 Project Overview
This project is a high-performance **C++ Instant Messaging (IM) Backend** that evolved from a web server. It uses a **Reactor pattern** with Linux Epoll for non-blocking I/O and **Google Protobuf** for binary data serialization.

### 🏗 Architecture
- **Core:** Single-threaded Reactor (Event Loop) dispatching tasks to a Thread Pool.
- **I/O Model:** Non-blocking I/O + Epoll (ET mode).
- **Concurrency:** Fixed-size Thread Pool for computing tasks (business logic).
- **Database:** MySQL with connection pooling, accessed via `sqlpp11` (Modern C++ ORM).
- **Protocol:** Custom TCP protocol with Protobuf payloads.
- **Auth:** JWT-based authentication.

### 🛠 Tech Stack
- **Language:** C++20
- **Build System:** CMake + Vcpkg
- **Third-party Libraries:**
  - `protobuf`: Serialization
  - `sqlpp11`: Type-safe SQL embedding
  - `jwt-cpp`: JWT generation/verification
  - `nlohmann/json`: JSON parsing (auxiliary)
  - `openssl`: Encryption

---

## 🚀 Building and Running

### Prerequisites
- Docker & VS Code DevContainer (Recommended)
- OR: GCC 11+/Clang 14+, CMake 3.21+, Vcpkg

### Build Commands
The project uses CMake Presets.

**Debug Build:**
```bash
cmake --preset debug
cmake --build build/debug
```

**Release Build:**
```bash
cmake --preset release
cmake --build build/release
```

**Database Model Generation:**
Changing `sql/schema.sql` requires a rebuild to regenerate C++ models (`server/src/model/schema.h`).
```bash
# Handled automatically by CMake via `ddl2cpp` script
cmake --build build/debug --target generate_sql_models
```

### Running the Server
```bash
./build/debug/server/src/server
```
*Port:* 1316 (Default)
*DB:* testdb (User: root, Pass: 123456)

---

## 🧪 Testing

### Unit/Integration Tests
The project uses Python scripts for functional testing of the TCP interface.

```bash
# Test Authentication (Register/Login)
python3 tests/test_auth.py [username] [password]

# Test Friend System
python3 tests/test_friend.py
```

---

## 📂 Key Directories

- `server/src/core/`: Core reactor implementation (`Epoller`, `Webserver`, `TcpConnection`).
- `server/src/service/`: Business logic (Auth, Friend, etc.).
- `server/src/dao/`: Data Access Objects using `sqlpp11`.
- `server/src/model/`: Generated C++ structs for DB tables.
- `proto/`: Protocol Buffer definitions.
- `sql/`: Database schema (`schema.sql`).
- `tests/`: Python test scripts.

## 📝 Conventions
- **Naming:**
  - Classes: `CamelCase`
  - Functions/Variables: `snake_case`
  - Member variables: `variable_` (trailing underscore)
- **Formatting:** `clang-format` (file provided).
- **Logging:** Async logger used throughout the codebase.
