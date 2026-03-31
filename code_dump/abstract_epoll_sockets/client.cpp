#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <cstddef> // for offsetof

#define SOCKET_NAME "\0my_abstract_socket_name"

int main() {
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    
    const size_t name_len = sizeof(SOCKET_NAME);
    memcpy(addr.sun_path, SOCKET_NAME, name_len);

    // Generic length calculation
    socklen_t addr_len = offsetof(struct sockaddr_un, sun_path) + name_len;

    if (connect(sock_fd, (struct sockaddr*)&addr, addr_len) == -1) {
        perror("connect");
        return 1;
    }

    const char* msg = "Hello, generic abstract socket!";
    write(sock_fd, msg, strlen(msg));
    
    close(sock_fd);
    return 0;
}
