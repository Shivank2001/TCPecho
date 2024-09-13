#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#define BUFFER_SIZE 1024  // Increased buffer size for flexibility

// Function prototypes
void error_sys(const char *message);
void communicate(int socket1);

char bufferi[BUFFER_SIZE];  // Input buffer
char buffero[BUFFER_SIZE];  // Output buffer

int main(int argc, char *argv[]) {
    int socket1;
    struct sockaddr_in serveraddr;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create socket
    socket1 = socket(AF_INET, SOCK_STREAM, 0);
    if (socket1 < 0) {
        error_sys("Socket creation failed");
    }
    printf("Socket created\n");

    // Setup server address structure
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) <= 0) {
        error_sys("Invalid IP address");
    }

    // Connect to the server
    if (connect(socket1, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        error_sys("Connection failed");
    }
    printf("Connected to server\n");

    communicate(socket1);

    close(socket1);
    return 0;
}

void communicate(int socket1) {
    ssize_t bytesread, byteswritten;

    while (1) {
        // Read user input
        memset(bufferi, 0, sizeof(bufferi));
        if (fgets(bufferi, sizeof(bufferi), stdin) == NULL) {
            if (feof(stdin)) {
                printf("EOF detected. Exiting...\n");
                break;
            }
            error_sys("Error reading input");
        }

        // Remove newline character if present
        size_t len = strlen(bufferi);
        if (len > 0 && bufferi[len - 1] == '\n') {
            bufferi[len - 1] = '\0';
        }

        // Write to socket
        byteswritten = write(socket1, bufferi, strlen(bufferi));
        if (byteswritten < 0) {
            error_sys("Write failed");
        }
        printf("Bytes written: %zd\n", byteswritten);

        // Read from socket
        memset(buffero, 0, sizeof(buffero));
        bytesread = read(socket1, buffero, sizeof(buffero) - 1);
        if (bytesread < 0) {
            error_sys("Read failed");
        } else if (bytesread == 0) {
            printf("Server disconnected\n");
            break;
        }

        // Null-terminate and print received data
        buffero[bytesread] = '\0';
        printf("Echo from server: %s\n", buffero);
        printf("Bytes read: %zd\n", bytesread);
    }
}

void error_sys(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}
