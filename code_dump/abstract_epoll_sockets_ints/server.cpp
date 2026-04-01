// ... [Headers same as your original file] ...
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <cstddef>

#define SOCKET_NAME "\0my_abstract_socket_name"
#define MAX_EVENTS 10

// 1. Must match the Client's struct definition
struct Packet {
    int val1;
    int val2;
    int val3;
    int val4;
};

int main() {
    // ... [Socket setup and epoll_ctl logic remains the same] ...
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    const size_t name_len = sizeof(SOCKET_NAME); 
    memcpy(addr.sun_path, SOCKET_NAME, name_len);
    socklen_t addr_len = offsetof(struct sockaddr_un, sun_path) + name_len;

    bind(server_fd, (struct sockaddr*)&addr, addr_len);
    listen(server_fd, 5);

    int epoll_fd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    std::cout << "Server listening for integer packets..." << std::endl;

    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int n = 0; n < nfds; ++n) {
            int fd = events[n].data.fd;

            if (fd == server_fd) {
                int client_fd = accept(server_fd, NULL, NULL);
                struct epoll_event client_ev;
                client_ev.events = EPOLLIN;
                client_ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);
            } 
            else {
                // 2. Read directly into a Packet object
                Packet received_data;
                ssize_t bytes = read(fd, &received_data, sizeof(Packet));
                
                if (bytes <= 0) {
                    std::cout << "Client disconnected." << std::endl;
                    close(fd);
                } else if (bytes == sizeof(Packet)) {
                    // 3. Access members directly
                    std::cout << "Received integers: " 
                              << received_data.val1 << ", " 
                              << received_data.val2 << ", " 
                              << received_data.val3 << ", " 
                              << received_data.val4 << std::endl;
                }
            }
        }
    }
    return 0;
}
