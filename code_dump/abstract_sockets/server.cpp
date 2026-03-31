#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

int main() {
    int server_fd, client_fd;
    struct sockaddr_un address;
    const char* socket_path = "\0my_abstract_socket"; // Abstract Namespace

    // 1. Create socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    // 2. Bind socket
    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, socket_path, sizeof(address.sun_path) - 1);
    
    // Size includes the null byte + path length
    bind(server_fd, (struct sockaddr*)&address, sizeof(sa_family_t) + strlen(socket_path + 1) + 1);

    // 3. Listen
    listen(server_fd, 5);
    std::cout << "Server listening..." << std::endl;

    // 4. Accept
    client_fd = accept(server_fd, nullptr, nullptr);
    
    // Read data
    char buffer[1024] = {0};
    read(client_fd, buffer, 1024);
    std::cout << "Received: " << buffer << std::endl;

    close(client_fd);
    close(server_fd);
    return 0;
}
