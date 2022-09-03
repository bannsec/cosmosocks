
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

STATIC_YOINK("__die");
STATIC_YOINK("malloc_inspect_all");
STATIC_YOINK("__get_symbol_by_addr");

uint8_t num_methods = 0;

// Address type struct
typedef struct {
    uint8_t atype;
    uint8_t *addr;
    uint8_t addr_size;
    uint16_t port;
} socks5_address;

#include "common.c"
#include "auth.h"

extern auth Authentication;

uint8_t *get_requested_auth_methods(int sock) {
    if ( !read_check(sock, &num_methods, 1, "read num_methods") ) return NULL;

    uint8_t *methods = malloc(num_methods);
    if ( !read_check(sock, methods, num_methods, "read methods") ) {
        free(methods);
        return NULL;
    }
    return methods;
}

// Function: authenticate_user_auth_username_password
// Description: Authenticates the user with username and password
// Steps:
//   1. Read one byte from the socket, this is the Version field, must be 0x01
//   2. Read one byte from the socket, this is the IDLen field. It is uint8_t
//   3. Read the IDLen bytes from the socket, this is the ID field (char *)
//   4. Read one byte from the socket, this is the PasswordLen field. It is uint8_t
//   5. Read the PasswordLen bytes from the socket, this is the Password field (char *)
// Input:
//   int sock - the socket to read from
// Output:
//   bool - true if the user was authenticated, false otherwise
bool authenticate_user_auth_username_password(int sock) {
    uint8_t version = 0;
    uint8_t id_len = 0;
    uint8_t password_len = 0;
    bool success = false;

    char *id = NULL;
    char *password = NULL;
    if ( !read_check(sock, &version, 1, "read version") ) goto beach;

    if ( version != 0x01 ) {
        fprintf(stderr, "Invalid version\n");
        goto beach;
    }
    if ( !read_check(sock, &id_len, 1, "read id_len") ) goto beach;
    id = malloc(id_len + 1);
    if ( !read_check(sock, id, id_len, "read id") ) {
        goto beach;
    }
    id[id_len] = '\0';
    if ( !read_check(sock, &password_len, 1, "read password_len") ) {
        goto beach;
    }
    password = malloc(password_len + 1);
    if ( !read_check(sock, password, password_len, "read password") ) {
        goto beach;
    }
    password[password_len] = '\0';
    if (strcmp(id, Authentication.data->userpass->username) == 0 && strcmp(password, Authentication.data->userpass->password) == 0) {
        success = true;
        goto beach;
    }
    printf("Invalid username or password\n");

    beach:
    free(id);
    free(password);
    return success;
}

bool authenticate_user(int sock, uint8_t *methods) {
    uint8_t response[2] = {0x05, 0x00};
    int ver = 1;
    int status = 0;

    for (int i = 0; i < num_methods; i++) {
        if (Authentication.type == AUTH_NONE && methods[i] == 0) {
            // Tell client we agree
            if ( !write_check(sock, response, 2, "write method authenticate_user") ) return false;
            return true;
        } else if (Authentication.type == AUTH_USERPASS && methods[i] == 2) {
            // Tell client we agree. We should get auth next
            response[1] = 2;
            if ( !write_check(sock, response, 2, "write method authenticate_user") ) return false;
            if ( authenticate_user_auth_username_password(sock) ) {
                // Auth was good
                status = 0;
                if ( !write_check(sock, &ver, 1, "write method authenticate_user") ) return false;
                if ( !write_check(sock, &status, 1, "write method authenticate_user") ) return false;
                return true;
            } else {
                // Auth was bad
                status = 1;
                if ( !write_check(sock, &ver, 1, "write method authenticate_user") ) return false;
                if ( !write_check(sock, &status, 1, "write method authenticate_user") ) return false;
            }
        }
    }
    response[1] = 0xFF;
    write_check(sock, response, 2, "write method authenticate_user");
    return false;
}

socks5_address *read_socks5_address(int sock) {
    int n;
    socks5_address *addr = malloc(sizeof(socks5_address));
    if ( !read_check(sock, &addr->atype, 1, "read atype") ) return NULL;

    switch (addr->atype) {
        case 1:
            addr->addr = malloc(4);
            if ( !read_check(sock, addr->addr, 4, "read ip") ) {
                free(addr->addr);
                free(addr);
                return NULL;
            }
            break;
        case 3:
            if ( !read_check(sock, &addr->addr_size, 1, "read addr_size") ) {
                free(addr);
                return NULL;
            }
            addr->addr = malloc(addr->addr_size+1);
            if ( !read_check(sock, addr->addr, addr->addr_size, "read domain") ) {
                free(addr->addr);
                free(addr);
                return NULL;
            }
            addr->addr[addr->addr_size] = '\0';
            break;
        case 4:
            addr->addr = malloc(16);
            if ( !read_check(sock, addr->addr, 16, "read ipv6") ) {
                free(addr->addr);
                free(addr);
                return NULL;
            }
            break;
        default:
            return NULL;
    }
    if ( !read_check(sock, &addr->port, 2, "read port") ) {
        free(addr->addr);
        free(addr);
        return NULL;
    }
    return addr;
}

bool send_socks5_response(int sock, uint8_t status, socks5_address *address) {
    uint8_t response[3] = {0x05, status, 0x00};
    int n;

    if ( !write_check(sock, response, 3, "write response") ) return false;

    switch (address->atype) {
        case 1:
            if ( !write_check(sock, &address->atype, 1, "write atype") ) return false;
            if ( !write_check(sock, address->addr, 4, "write addr") ) return false;
            break;
        case 3:
            if ( !write_check(sock, &address->atype, 1, "write atype") ) return false;
            if ( !write_check(sock, &address->addr_size, 1, "write addr_size") ) return false;
            if ( !write_check(sock, address->addr, address->addr_size, "write addr") ) return false;
            break;
        case 4:
            if ( !write_check(sock, &address->atype, 1, "write atype") ) return false;
            if ( !write_check(sock, address->addr, 16, "write addr") ) return false;
            break;
        default:
            return false;
    }

    if ( !write_check(sock, &address->port, 2, "write port") ) return false;
    return true;    
}

bool handle_connect_command(int sock) {
    socks5_address *addr = read_socks5_address(sock);
    int new_sock;
    if (addr == NULL) return false;

    // If address is IPv4
    if (addr->atype == 1) {
        new_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (new_sock < 0) {
            perror("socket");
            return false;
        }

        // Connect forward
        struct sockaddr_in forward_addr;
        forward_addr.sin_family = AF_INET;
        forward_addr.sin_port = addr->port;
        forward_addr.sin_addr.s_addr = *((uint32_t *)addr->addr);
        int n = connect(new_sock, (struct sockaddr *)&forward_addr, sizeof(forward_addr));
        if (n < 0) {
            perror("connect");
            return false;
        }

        // Send success response
        if ( !send_socks5_response(sock, 0x00, addr) ) return false;

        return connect_sockets(sock, new_sock);
    } else if ( addr->atype == 3) {
        // Resolve hostname to IP address in network byte order
        struct hostent *host = gethostbyname((char *)addr->addr);
        if (host == NULL) {
            perror("gethostbyname");
            return false;
        } else if (host->h_addrtype != AF_INET) {
            perror("gethostbyname");
            return false;
        } else if (host->h_addr_list[0] == NULL) {
            perror("gethostbyname");
            return false;
        } else {
            new_sock = socket(AF_INET, SOCK_STREAM, 0);
            if (new_sock < 0) {
                perror("socket");
                return false;
            }
            // Connect forward
            struct sockaddr_in forward_addr;
            forward_addr.sin_family = AF_INET;
            forward_addr.sin_port = addr->port;
            forward_addr.sin_addr.s_addr = *((uint32_t *)host->h_addr_list[0]);
            int n = connect(new_sock, (struct sockaddr *)&forward_addr, sizeof(forward_addr));
            if (n < 0) {
                perror("connect");
                return false;
            }
            // Send success response
            if ( !send_socks5_response(sock, 0x00, addr) ) return false;
            return connect_sockets(sock, new_sock);
        }
    }

    else send_socks5_response(sock, 0x08, addr);

}

bool handle_new_socks5_connection(int sockfd) {
    uint8_t version, n, command, reserved;
    uint8_t *methods = get_requested_auth_methods(sockfd);
    if (methods == NULL) {
        return false;
    }

    if (!authenticate_user(sockfd, methods)) {
        return false;
    }

    free(methods);

    if (!read_check(sockfd, &version, 1, "read version")) return false;

    if (version != 0x05) {
        printf("Invalid version: %d\n", version);
        return false;
    }

    if (!read_check(sockfd, &command, 1, "read command")) return false;
    if (!read_check(sockfd, &reserved, 1, "read reserved")) return false;

    if (reserved != 0x00) {
        printf("Invalid reserved: %d\n", reserved);
        return false;
    }

    if (command == 0x01) {
        printf("Connect command\n");
        return handle_connect_command(sockfd);
    } else if (command == 0x02) {
        printf("Bind command\n");
        //return handle_bind_command(sockfd);
    } else if (command == 0x03) {
        printf("UDP associate command\n");
        //return handle_udp_associate_command(sockfd);
    } else {
        printf("Invalid command: %d\n", command);
        return false;
    }

    return true;
}
