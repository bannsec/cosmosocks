/*
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
*/

// https://en.wikipedia.org/wiki/SOCKS

typedef struct {
    char version;
    char command;
    uint16_t port;
    uint32_t ip;
} socks4_request;

typedef struct {
    char version;
    char reply;
    uint16_t port;
    uint32_t ip;
} socks4_response;

#include "common.c"

// Function: read_socks4_request
// Description: Reads a SOCKS4 request from the socket.
//              Assumes protocol version has already been read and is 4
// Input:
//   int sock - the socket to read from
// Output:
//   bool - true if the request was read successfully, false otherwise
static bool read_socks4_request(int sock, socks4_request *request) {
    int n = read(sock, &request->command, 1);
    if (n < 0) {
        perror("read");
        return false;
    }
    if (n == 0) {
        return false;
    }
    n = read(sock, &request->port, 2);
    if (n < 0) {
        perror("read");
        return false;
    }
    if (n == 0) {
        return false;
    }
    n = read(sock, &request->ip, 4);
    if (n < 0) {
        perror("read");
        return false;
    }
    if (n == 0) {
        return false;
    }

    // Read until we reach the null byte, ignoring for now
    char c;
    do {
        n = read(sock, &c, 1);
        if (n < 0) {
            perror("read");
            return false;
        }
        if (n == 0) {
            return false;
        }
    } while (c != 0);


    return true;
}

// Function: write_socks4_response
// Description: Writes a SOCKS4 response to the socket.
// Parameters:
//   int sock - the socket to write to
//   socks4_response response - the response to write
// Returns: boolean indicating success or failure
static bool write_socks4_response(int sock, socks4_response *response) {
    // Write the version
    int n = write(sock, &response->version, 1);
    if (n < 0) {
        perror("write");
        return false;
    }
    if (n == 0) {
        return false;
    }

    n = write(sock, &response->reply, 1);
    if (n < 0) {
        perror("write");
        return false;
    }
    if (n == 0) {
        return false;
    }
    n = write(sock, &response->port, 2);
    if (n < 0) {
        perror("write");
        return false;
    }
    if (n == 0) {
        return false;
    }
    n = write(sock, &response->ip, 4);
    if (n < 0) {
        perror("write");
        return false;
    }
    if (n == 0) {
        return false;
    }
    return true;
}

// Function: send_socks4_response
// Description: Sends a SOCKS4 response to the socket.
// Parameters:
//   int sock - the socket to write to
//   char reply - the reply to send
//   char port[2] - the port to send
//   char ip[4] - the ip to send
// Returns: boolean indicating success or failure
static bool send_socks4_response(int sock, char reply, uint16_t port, uint32_t ip) {
    socks4_response response;
    response.version = 0;
    response.reply = reply;
    memcpy(&response.port, &port, 2);
    memcpy(&response.ip, &ip, 4);
    return write_socks4_response(sock, &response);
}

// Function: handle_socks4_connect
// Description: Handles a SOCKS4 connect request.
//              Attempts to connect to the specified host and port.
//              Sends a SOCKS4 response to the client.
// Parameters:
//   int sock - the socket to read from
//   socks4_request *request - the request to handle
// Returns: boolean indicating success or failure
static bool handle_socks4_connect(int sock, socks4_request *request) {
    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }
    // Set the socket to non-blocking
    //int flags = fcntl(sockfd, F_GETFL, 0);
    //if (flags < 0) {
    //    perror("fcntl");
    //    return false;
    //}
    //flags |= O_NONBLOCK;
    //
    //if (fcntl(sockfd, F_SETFL, flags) < 0) {
    //    perror("fcntl");
    //    return false;
    //}

    // Set socket to TCP_NODELAY
    //int one = 1;
    //if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) < 0) {
    //    perror("setsockopt");
    //    return false;
    //}

    // Connect to the host
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = request->port;
    addr.sin_addr.s_addr = request->ip;
    int n = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (n < 0) {
        if (errno != EINPROGRESS) {
            perror("connect");
            return false;
        }
    }
    // Wait for the connection to complete
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    n = select(sockfd + 1, NULL, &fds, NULL, &tv);
    if (n < 0) {
        perror("select");
        return false;
    }
    if (n == 0) {
        printf("Connection timed out\n");
        return false;
    }
    // Check the socket for errors
    int error;
    socklen_t len = sizeof(error);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        perror("getsockopt");
        return false;
    }
    if (error != 0) {
        printf("Error connecting to host: %s\n", strerror(error));
        return false;
    }
    // Send a SOCKS4 response
    send_socks4_response(sock, 0x5a, request->port, request->ip);

    // Connect the sockets
    return connect_sockets(sock, sockfd);
}

// Function: handle_new_socks4_connection
// Description: Handles a new connection to the SOCKS server.
//              This function will read the SOCKS request from the client,
//              and then send a SOCKS response back to the client.
// Parameters:
//  - int sockfd: The socket file descriptor for the new connection.
// Returns:
//  - true if the connection was successfully handled, false otherwise.
bool handle_new_socks4_connection(int sockfd) {
    // Read the SOCKS request from the client
    socks4_request request;
    if (!read_socks4_request(sockfd, &request)) {
        return false;
    }

    // If the request command is 1, then call handle_socks4_connect()
    // If the request command is 2, then call handle_socks4_bind()
    if (request.command == 1) {
        return handle_socks4_connect(sockfd, &request);
    } else if (request.command == 2) {
        close(sockfd);
        return false;
        //return handle_socks4_bind(sockfd, &request);
    } else {
        // Unknown command
        close(sockfd);
        return false;
    }
    
    return true;
}