#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include <cstring>

#define SOCKET_PATH "/tmp/uds_example.sock"
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, epoll_fd;
    struct sockaddr_un addr;

    // 1. Create Socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // Clean up old socket file if it exists
    unlink(SOCKET_PATH);

    // 2. Bind
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return 1;
    }

    // 3. Listen
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        return 1;
    }

    // 4. Setup Epoll
    epoll_fd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN; // Watch for input
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    std::cout << "Server listening on " << SOCKET_PATH << "..." << std::endl;

    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                // Handle new connection
                int client_fd = accept(server_fd, NULL, NULL);
                std::cout << "Accepted new connection. Client FD: " << client_fd << std::endl;
                
                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
            } else {
                // Handle data from client
                char buffer[BUFFER_SIZE];
                int bytes_read = read(events[n].data.fd, buffer, sizeof(buffer) - 1);
                
                if (bytes_read <= 0) {
                    std::cout << "Client disconnected (FD: " << events[n].data.fd << ")" << std::endl;
                    close(events[n].data.fd);
                } else {
                    buffer[bytes_read] = '\0';
                    std::cout << "Received from FD " << events[n].data.fd << ": " << buffer << std::endl;
                }
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
