#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <cstddef>

#define SOCKET_NAME "\0my_abstract_socket_name"

// 1. Define the data structure
struct Packet {
    int val1;
    int val2;
    int val3;
    int val4;
};

template <typename T>
bool send_packet(int fd, const T& data) {
    ssize_t bytes_sent = write(fd, &data, sizeof(T));

    if (bytes_sent == -1) {
        perror("write");
        return false;
    } else if (static_cast<size_t>(bytes_sent) < sizeof(T)) {
        // Handle partial writes if necessary for large buffers
        std::cerr << "Warning: Partial write occurred" << std::endl;
        return false;
    }

    return true;
}

int main() {
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    
    const size_t name_len = sizeof(SOCKET_NAME);
    memcpy(addr.sun_path, SOCKET_NAME, name_len);

    socklen_t addr_len = offsetof(struct sockaddr_un, sun_path) + name_len;

    if (connect(sock_fd, (struct sockaddr*)&addr, addr_len) == -1) {
        perror("connect");
        return 1;
    }

    // 2. Prepare the data
    Packet data = {10, 20, 30, 40};

    // // 3. Send the entire struct in one go
    // if (write(sock_fd, &data, sizeof(data)) == -1) {
    //     perror("write");
    // } else {
    //     std::cout << "Sent 4 integers to server." << std::endl;
    // }
    if (send_packet(sock_fd, data)) {
        std::cout << "Successfully sent packet via send_packet function." << std::endl;
    }    
    
    close(sock_fd);
    return 0;
}
