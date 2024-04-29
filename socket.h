#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>

class Socket
{
public:
    Socket();
    ~Socket();

    // Initialize the socket with the specified port and host
    bool init(int port, const std::string &host);

    // Set the socket to non-blocking mode
    void setNonBlocking();

    // Close the socket
    void closeSocket();

    // Listen for incoming connections
    bool listenOnSocket(int backlog = 5);

    // Accept a new connection
    int acceptConnection(struct sockaddr_in &clientAddr);

    // Read data from the socket
    ssize_t readData(int sockfd, void *buf, size_t len);

    // Write data to the socket
    ssize_t writeData(int sockfd, const void *buf, size_t len);

    // Get the file descriptor of the socket
    int getSocketFD() const;

    // Poll the socket for reading or writing
    int pollSocket(struct pollfd *fds, nfds_t nfds, int timeout);

private:
    int sockfd;              // Socket file descriptor
    struct sockaddr_in addr; // Socket address structure

    // Disable copying and assignment
    Socket(const Socket &);
    Socket &operator=(const Socket &);
};

#endif // SOCKET_H
