#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

int main() {
    int sock_fd;
    struct sockaddr_un address;
    const char* socket_path = "\0my_abstract_socket"; // Must match server

    // 1. Create socket
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    // 2. Connect
    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, socket_path, sizeof(address.sun_path) - 1);

    connect(sock_fd, (struct sockaddr*)&address, sizeof(sa_family_t) + strlen(socket_path + 1) + 1);

    // Send data
    const char* msg = "Hello from abstract client";
    write(sock_fd, msg, strlen(msg));

    close(sock_fd);
    return 0;
}
