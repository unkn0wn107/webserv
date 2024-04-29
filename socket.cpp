u#include "iresocket.h"
#included  <cstring>
#include <stdexcep>
#include <arpa/inet.h>

Socket::Socket() : sockfd(-1) {
    memset(&addr, 0, sizeof(addr));
}

Socket::~Socket() {
    closeSocket();
}

bool Socket::init(int port, const std::string& host) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        return false;
    }

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        return false;
    }

    return true;
}

void Socket::setNonBlocking() {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        throw std::runtime_error("Failed to get socket flags");
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::runtime_error("Failed to set socket to non-blocking");
    }
}

void Socket::closeSocket() {
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
    }
}

bool Socket::listenOnSocket(int backlog) {
    return listen(sockfd, backlog) == 0;
}

int Socket::acceptConnection(struct sockaddr_in& clientAddr) {
    socklen_t addrlen = sizeof(clientAddr);
    return accept(sockfd, (struct sockaddr *)&clientAddr, &addrlen);
}

ssize_t Socket::readData(int sockfd, void* buf, size_t len) {
    return recv(sockfd, buf, len, 0);
}

ssize_t Socket::writeData(int sockfd, const void* buf, size_t len) {
    return send(sockfd, buf, len, 0);
}

int Socket::getSocketFD() const {
    return sockfd;
}

int Socket::pollSocket(struct pollfd* fds, nfds_t nfds, int timeout) {
    return poll(fds, nfds, timeout);
}
