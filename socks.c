
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

#include "socks4.h"
#include "socks5.h"


// Function: handle_new_server_connection
// Description:
//   Reads first byte of the request and decides what to do with the connection.
// Input:
//   int sock - the socket on which the request was received
static void handle_new_server_connection(int sock) {
    char version;
    int n = read(sock, &version, 1);
    if (n < 0) {
        perror("read");
        return;
    }
    if (n == 0) {
        return;
    }
    if (version == 4) {
        handle_new_socks4_connection(sock);
    } else if (version == 5) {
        handle_new_socks5_connection(sock);
    } else {
        //fprintf(stderr, "Unsupported SOCKS version: %d\n", version);
        close(sock);
    }
}

// Function: start_socks_server
// Description: Starts the SOCKS server on the specified port and ip address.
//              The server will listen for incoming connections and spawn a new thread for each connection.
//              The thread will handle the connection and then exit.
// Parameters:
//  - port: the port to listen on
//  - ip: the ip address to listen on
// Returns: boolean indicating success or failure
bool start_socks_server(int port, char *ip) {
    // Tell the system we won't be reaping zombies
    signal(SIGCHLD, SIG_IGN);

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }
    
    // Set the socket to be reusable
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        return false;
    }
    
    // Bind the socket to the specified ip and port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return false;
    }
    
    // Listen for incoming connections
    if (listen(sockfd, 5) < 0) {
        perror("listen");
        return false;
    }
    
    // Accept incoming connections and spawn a new thread for each one
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sockfd < 0) {
            perror("accept");
            return false;
        }
        
        /*
        // Spawn a new thread for the client
        pthread_t thread;
        if (pthread_create(&thread, NULL, &handle_new_server_connection, (void *)client_sockfd) != 0) {
            perror("pthread_create");
            return false;
        }
        */

        // Fork a new process for the client
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return false;
        } else if (pid == 0) {
            // Child process
            handle_new_server_connection(client_sockfd);
            exit(0);
        } else {
            // Parent process
            close(client_sockfd);
        }
    }
    
    return true;
}