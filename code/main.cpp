#include <unistd.h>
#include "server/webserver.h"

int main() {
    Webserver server(
        1316, 3, 60000,
        3306, "root", "13456", "testdb",
        12, 8, true, 1, 1024);
    server.start();
} 