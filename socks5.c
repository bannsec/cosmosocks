
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

uint8_t num_methods = 0;

// Address type struct
typedef struct {
    uint8_t atype;
    uint8_t *addr;
    uint16_t port;
} socks5_address;

#include "common.c"

uint8_t *get_requested_auth_methods(int sock) {
    int n = read(sock, &num_methods, 1);
    if (n < 0) {
        perror("read num_methods");
        return NULL;
    }
    if (n == 0) return NULL;

    uint8_t *methods = malloc(num_methods);
    n = read(sock, methods, num_methods);
    if (n < 0) {
        perror("read methods");
        return NULL;
    }
    if (n == 0) return NULL;
    return methods;
}

bool authenticate_user(int sock, uint8_t *methods) {
    uint8_t response[2] = {0x05, 0x00};
    int n;

    // Only allowing no authentication for now
    for (int i = 0; i < num_methods; i++) {
        if (methods[i] == 0) {
            n = write(sock, response, 2);
            if (n < 0) {
                perror("write method authenticate_user");
                return false;
            }
            return true;
        }
    }
    response[1] = 0xFF;
    n = write(sock, response, 2);
    if (n < 0) {
        perror("write method authenticate_user");
        return false;
    }

    return false;
}

socks5_address *read_socks5_address(int sock) {
    socks5_address *addr = malloc(sizeof(socks5_address));
    int n = read(sock, &addr->atype, 1);
    if (n < 0) {
        perror("read atype");
        return NULL;
    }
    if (n == 0) return NULL;
    switch (addr->atype) {
        case 1:
            addr->addr = malloc(4);
            n = read(sock, addr->addr, 4);
            if (n < 0) {
                perror("read addr");
                return NULL;
            }
            if (n == 0) return NULL;
            break;
        case 3:
            addr->addr = malloc(1);
            n = read(sock, addr->addr, 1);
            if (n < 0) {
                perror("read addr");
                return NULL;
            }
            if (n == 0) return NULL;
            addr->addr = realloc(addr->addr, addr->addr[0] + 1);
            n = read(sock, addr->addr + 1, n);
            if (n < 0) {
                perror("read addr");
                return NULL;
            }
            if (n == 0) return NULL;
            break;
        case 4:
            addr->addr = malloc(16);
            n = read(sock, addr->addr, 16);
            if (n < 0) {
                perror("read addr");
                return NULL;
            }
            if (n == 0) return NULL;
            break;
        default:
            return NULL;
    }
    n = read(sock, &addr->port, 2);
    if (n < 0) {
        perror("read port");
        return NULL;
    }
    if (n == 0) return NULL;
    return addr;
}

bool send_socks5_response(int sock, uint8_t status, socks5_address *address) {
    uint8_t response[3] = {0x05, status, 0x00};
    int n;

    n = write(sock, response, 3);
    if (n < 0) {
        perror("write response");
        return false;
    }
    if (n == 0) return false;

    switch (address->atype) {
        case 1:
            n = write(sock, &address->atype, 1);
            if (n < 0) {
                perror("write atype");
                return false;
            }
            if (n == 0) return false;
            n = write(sock, address->addr, 4);
            if (n < 0) {
                perror("write addr");
                return false;
            }
            if (n == 0) return false;
            break;
        case 3:
            n = write(sock, &address->atype, 1);
            if (n < 0) {
                perror("write atype");
                return false;
            }
            if (n == 0) return false;
            n = write(sock, &address->addr[0], 1);
            if (n < 0) {
                perror("write addr");
                return false;
            }
            if (n == 0) return false;
            n = write(sock, address->addr[1], address->addr[0]);
            if (n < 0) {
                perror("write addr");
                return false;
            }
            if (n == 0) return false;
            break;
        case 4:
            n = write(sock, &address->atype, 1);
            if (n < 0) {
                perror("write atype");
                return false;
            }
            if (n == 0) return false;
            n = write(sock, address->addr, 16);
            if (n < 0) {
                perror("write addr");
                return false;
            }
            if (n == 0) return false;
            break;
        default:
            return false;
    }

    n = write(sock, &address->port, 2);
    if (n < 0) {
        perror("write port");
        return false;
    }
    if (n == 0) return false;
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
        puts("Domain name not supported");
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

    n = read(sockfd, &version, 1);
    if (n < 0) {
        perror("read version");
        return false;
    }
    if (n == 0) return false;
    if (version != 0x05) {
        printf("Invalid version: %d\n", version);
        return false;
    }

    n = read(sockfd, &command, 1);
    if (n < 0) {
        perror("read command");
        return false;
    }
    if (n == 0) return false;

    n = read(sockfd, &reserved, 1);
    if (n < 0) {
        perror("read reserved");
        return false;
    }
    if (n == 0) return false;
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