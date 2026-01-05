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

1. Implementing user registration and login
2. Implementing user adding friend and removing friend
3. Implementing user sending message to friend

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

**Run:**
```bash
./build/release/server/src/server
```

---

## 🚀 Quick Start (Docker Compose)

If you just want to run the server without setting up a development environment:

```bash
docker-compose up --build
```

**Code Formatting:**
```bash
find server/src test -name "*.h" -o -name "*.cpp" | xargs clang-format -i
```

*Note: The server currently listens on port 1316.*

## 📂 Directory Structure
```
.
├── proto           # Protobuf definitions
├── server
│   ├── src         # Server source code (main, http, log, pool, etc.)
│   └── log         # Runtime logs
├── resources       # Static resources (HTML, CSS, JS, etc.)
├── test            # Unit Tests
├── .devcontainer   # DevContainer configuration
├── docker-compose.yml
└── README.md
```
