
#include "socks.h"

#define DEFAULT_PORT 1080
#define DEFAULT_IP "0.0.0.0"

void print_help() {
    printf("Usage: cosmosocks_server [OPTIONS]\n");
    printf("Options:\n");
    printf("  -h                 Print this help message\n");
    printf("  -p PORT            Set the port to listen on (default: %d)\n", DEFAULT_PORT);   
}

int main(int argc, char **argv, char **envp)
{
    
    /* 
        Use getopt to parse the command line arguments.
        -h: print help message and exit
        -p: specify the port number to listen on (default: 1080)
        -l: specify ip address to listen on (default: 0.0.0.0)
    */
    int c;
    int port = DEFAULT_PORT;
    char *ip = DEFAULT_IP;
    while ((c = getopt(argc, argv, "hp:l:")) != -1) {
        switch (c) {
            case 'h':
                print_help();
                exit(0);
            case 'p':
                port = atoi(optarg);
                break;
            case 'l':
                ip = optarg;
                break;
            default:
                print_help();
                exit(1);
        }
    }

    // Start the server
    start_socks_server(port, ip);

    return 0;
}