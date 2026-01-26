# Project Context: MyWebserver (CLI IM System)

## Overview
This project is a high-performance **CLI Instant Messaging (IM) System** written in **C++20**, evolving from a web server foundation. It utilizes a Reactor pattern with **Linux Epoll** for networking, **Google Protobuf** for data serialization, and relies on **MySQL** and **ScyllaDB** for data persistence.

## Architecture
- **Language**: C++20
- **Network Model**: Linux Epoll (Reactor Pattern)
- **Serialization**: Google Protobuf
- **Database**: 
  - **MySQL**: User data, auth (port 3306)
  - **ScyllaDB**: Message storage (port 9042)
- **Client UI**: FTXUI (Terminal-based UI)
- **Build System**: CMake (Presets enabled) + Vcpkg
- **Orchestration**: Docker Compose (DevContainer)

## Directory Structure
- `server/src/`: Core server logic (Reactor, ThreadPool, Database logic).
  - `main.cpp`: Entry point. Initializes `Webserver` on port 1316.
- `client/`: CLI client application using FTXUI.
  - `main.cpp`: Entry point. Manages UI pages (Auth, Register, Main).
- `proto/`: Protobuf definition files (`.proto`).
- `tests/`: Python integration tests (`test_auth.py`, `test_friend.py`).
- `.devcontainer/`: Development environment configuration (Dockerfile, docker-compose).
- `sql/`: Database schema definitions.

## Build & Run

### 1. Build
The project uses **CMake Presets** (`debug`, `release`).

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

### 2. Run Server
The server listens on port **1316**.
```bash
# Debug
./build/debug/server/src/server

# Release
./build/release/server/src/server
```

### 3. Run Client
The client connects to `127.0.0.1:1316`.
```bash
# Debug
./build/debug/client/client
```

## Testing
Python scripts are used for integration testing. They send raw Protobuf packets over TCP.

**Prerequisites:**
Ensure the server is running.

**Run Tests:**
```bash
# Test Authentication (Register/Login)
python3 tests/test_auth.py [username] [password]

# Test Friend System
python3 tests/test_friend.py

# Test Messaging
python3 tests/test_message.py
```

## Infrastructure
The project relies on Docker for database services.
- **MySQL**: 3306 (User/Auth)
- **Scylla**: 9042 (Messages)

**Start Infrastructure:**
```bash
docker-compose -f .devcontainer/docker-compose.yml up -d
```
(Or use the DevContainer which handles this automatically).

## Development Conventions
- **Code Style**: Google C++ Style (PascalCase for classes/functions, snake_case for variables).
- **Protobuf**: When modifying `.proto` files, re-run CMake to regenerate C++ and Python headers.
- **Database**: Schema changes require database re-initialization or migration.
