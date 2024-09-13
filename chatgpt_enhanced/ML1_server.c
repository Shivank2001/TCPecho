#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX 80
#define SA struct sockaddr

// Function to handle communication with the client
void func(int connfd) {
    char buff[MAX];
    ssize_t bytes_read;

    for (;;) {
        memset(buff, 0, sizeof(buff));  // Clear the buffer

        // Read message from client
        bytes_read = read(connfd, buff, sizeof(buff) - 1);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                printf("Client disconnected...\n");
            } else {
                perror("Read error");
            }
            break;
        }

        // Null-terminate and print the received message
        buff[bytes_read] = '\0';
        printf("From client: %s", buff);

        // Echo the received message back to the client
        if (write(connfd, buff, bytes_read) < 0) {
            perror("Write error");
            break;
        }
        printf("Echoed to client: %s", buff);
    }
}

int main(int argc, char *argv[]) {
    int sock, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, client;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number. Please specify a port between 1 and 65535.\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket successfully created.\n");

    // Prepare server address structure
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // Bind socket to address
    if (bind(sock, (SA*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Socket bind failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Socket successfully binded on port %d.\n", port);

    // Start listening for incoming connections
    if (listen(sock, 5) < 0) {
        perror("Listen failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d.\n", port);

    len = sizeof(client);

    while (1) {
        // Accept incoming connection
        connfd = accept(sock, (SA*)&client, &len);
        if (connfd < 0) {
            perror("Server accept failed");
            continue;
        }
        printf("Server accepted the client.\n");

        // Fork a new process to handle the client
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(connfd);
            continue;
        }

        if (pid == 0) {  // Child process
            close(sock);  // Close the listening socket in the child process
            func(connfd); // Handle client communication
            close(connfd); // Close the connected socket
            exit(EXIT_SUCCESS); // Exit the child process
        } else {  // Parent process
            close(connfd); // Close the connected socket in the parent process
        }

        // Wait for child processes to terminate to prevent zombie processes
        while (waitpid(-1, NULL, WNOHANG) > 0);
    }

    // Close the server socket (though this code is never reached)
    close(sock);

    return 0;
}
