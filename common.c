
// Function: connect_sockets
// Description: Connects two sockets, reads from socket1 and writes to socket2 and vice versa.
// Parameters:
//   int sock1 - the socket to read from
//   int sock2 - the socket to write to
// Returns: boolean indicating success or failure
static bool connect_sockets(int sock1, int sock2) {
    char buffer[1024] = {0};
    fd_set fds;
    int max_fd = sock1 > sock2 ? sock1 : sock2;
    
    // Timeout after 1 second
    //struct timeval timeout;
    //timeout.tv_sec = 1;
    //timeout.tv_usec = 0;
    

    while (true) {
        FD_ZERO(&fds);
        FD_SET(sock1, &fds);
        FD_SET(sock2, &fds);

        // Select on both sockets
        int n = select(max_fd + 1, &fds, NULL, NULL, NULL);
        if (n < 0) {
            perror("select");
            return false;
        }
        if (n == 0) {
            // Timeout
            continue;
        }
        if (FD_ISSET(sock1, &fds)) {
            // Read from sock1
            n = read(sock1, buffer, sizeof(buffer));
            if (n < 0) {
                perror("read");
                return false;
            }
            if (n == 0) {
                // Socket closed
                return true;
            }
            // Write to sock2
            n = write(sock2, buffer, n);
            if (n < 0) {
                perror("write");
                return false;
            }
            if (n == 0) {
                // Socket closed
                return true;
            }
        }

        if (FD_ISSET(sock2, &fds)) {
            // Read from sock2
            n = read(sock2, buffer, sizeof(buffer));
            if (n < 0) {
                perror("read");
                return false;
            }
            if (n == 0) {
                // Socket closed
                return true;
            }
            // Write to sock1
            n = write(sock1, buffer, n);
            if (n < 0) {
                perror("write");
                return false;
            }
            if (n == 0) {
                // Socket closed
                return true;
            }
        }
    }
    return true;
}

// Function: write_check
// Description: Performs write to a socket and checks for errors.
// Parameters:
//   int sock - the socket to write to
//   const char *buffer - the buffer to write
//   int length - the length of the buffer
//   const char *error - the error message to print if an error occurs
// Returns: boolean indicating success or failure
static bool write_check(int sock, const void *buffer, int length, const char *error) {
    int n = write(sock, buffer, length);
    if (n < 0) {
        perror(error);
        return false;
    }
    if (n != length) {
        fprintf(stderr, "write: wrote %d bytes, expected %d\n", n, length);
        return false;
    }
    return true;
}