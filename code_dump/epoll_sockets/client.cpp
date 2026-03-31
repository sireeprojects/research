#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

#define SOCKET_PATH "/tmp/uds_example.sock"

int main() {
    int sock_fd;
    struct sockaddr_un addr;

    // 1. Create Socket
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    // 2. Define Address
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // 3. Connect
    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // 4. Write Strings
    const char* messages[] = {"Hello from C++!", "Socket programming is cool.", "Goodbye!"};
    for (const char* msg : messages) {
        write(sock_fd, msg, strlen(msg));
        sleep(1); // Small delay to see output clearly on server
    }

    close(sock_fd);
    return 0;
}
