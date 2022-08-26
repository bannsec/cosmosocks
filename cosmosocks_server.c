
#include "socks.h"
#include "auth.h"

#define DEFAULT_PORT 1080
#define DEFAULT_IP "0.0.0.0"

auth Authentication = {0};

void print_help() {
    printf("Usage: cosmosocks_server [OPTIONS]\n");
    printf("Options:\n");
    printf("  -h                 Print this help message\n");
    printf("  -p PORT            Set the port to listen on (default: %d)\n", DEFAULT_PORT);   
    printf("  -U USERNAME        Set the username to use for authentication\n");
    printf("  -P PASSWORD        Set the password to use for authentication\n");
}

bool parse_authentication(char *username, char *password) {
    // If username is specified, then password must be specified
    if (username != NULL && password == NULL || username == NULL && password != NULL) {
        printf("Error: Username and password must be specified together\n");
        return false;
    }

    // Username/Password Auth
    if (username != NULL && password != NULL) {
        Authentication.type = AUTH_USERPASS;
        auth_userpass *userpass = malloc(sizeof(auth_userpass));
        userpass->username = username;
        userpass->password = password;
        auth_data *data = malloc(sizeof(auth_data));
        data->userpass = userpass;
        Authentication.data = data;
    } else {
        Authentication.type = AUTH_NONE;
        Authentication.data = NULL;
    }

    return true;
}

int main(int argc, char **argv, char **envp)
{
    
    /* 
        Use getopt to parse the command line arguments.
        -h: print help message and exit
        -p: specify the port number to listen on (default: 1080)
        -l: specify ip address to listen on (default: 0.0.0.0)
        -U: specify username for authentication (default: none)
        -P: specify password for authentication (default: none)
    */
    int c;
    int port = DEFAULT_PORT;
    char *ip = DEFAULT_IP;
    char *username = NULL;
    char *password = NULL;

    while ((c = getopt(argc, argv, "hp:l:U:P:")) != -1) {
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
            case 'U':
                username = optarg;
                break;
            case 'P':
                password = optarg;
                break;
            default:
                print_help();
                exit(1);
        }
    }


    if (!parse_authentication(username, password)) {
        exit(1);
    }

    // Start the server
    start_socks_server(port, ip);

    return 0;
}