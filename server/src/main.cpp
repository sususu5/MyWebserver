#include <signal.h>
#include <unistd.h>
#include "core/webserver.h"

static Webserver* g_server = nullptr;

void signal_handler(int sig) {
    const char msg[] = "\n[Signal] Received shutdown signal, stopping server...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    if (g_server) {
        g_server->Stop();
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);   // Ctrl+C
    sigaction(SIGTERM, &sa, nullptr);  // kill command

    {
        Webserver server(1316, 3, 60000, 3306, "root", "123456", "testdb", 50, 40, true, 1, 1024);
        g_server = &server;
        server.Start();
        g_server = nullptr;
        // Server destructor will be called here, logging shutdown
    }

    return 0;
}
