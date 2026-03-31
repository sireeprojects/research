#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>

// The name starts with \0. 
// sizeof() will count the leading \0 AND the trailing \0.
#define SOCKET_NAME "\0my_abstract_socket_name"
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket"); return 1; }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    
    // Generic length calculation:
    // sizeof(SOCKET_NAME) includes the leading \0 and the trailing \0.
    // For abstract sockets, the kernel just needs the total byte count.
    const size_t name_len = sizeof(SOCKET_NAME); 
    
    if (name_len > sizeof(addr.sun_path)) {
        std::cerr << "Socket name too long" << std::endl;
        return 1;
    }

    memcpy(addr.sun_path, SOCKET_NAME, name_len);

    // Generic address length for bind/connect
    socklen_t addr_len = offsetof(struct sockaddr_un, sun_path) + name_len;

    if (bind(server_fd, (struct sockaddr*)&addr, addr_len) == -1) {
        perror("bind"); return 1;
    }

    listen(server_fd, 5);

    int epoll_fd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    std::cout << "Server listening on abstract socket..." << std::endl;

    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int n = 0; n < nfds; ++n) {
            int fd = events[n].data.fd;

            if (fd == server_fd) {
                int client_fd = accept(server_fd, NULL, NULL);
                std::cout << "New Client FD: " << client_fd << std::endl;
                struct epoll_event client_ev;
                client_ev.events = EPOLLIN | EPOLLRDHUP;
                client_ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);
            } 
            else if (events[n].events & EPOLLIN) {
                char buffer[BUFFER_SIZE];
                ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
                
                if (bytes <= 0) {
                    std::cout << "Client " << fd << " disconnected." << std::endl;
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                } else {
                    buffer[bytes] = '\0';
                    std::cout << "Received: " << buffer << std::endl;
                }
            }
        }
    }
    return 0;
}
