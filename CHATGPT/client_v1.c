#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>

#define BUFFER_SIZE 1024

void start_client(const char *host, int port) {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    ssize_t total_bytes_sent = 0;
    ssize_t total_bytes_received = 0;

    // Create socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(host);
    server_addr.sin_port = htons(port);

    // Connect to server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Type messages to send (Ctrl+D to exit):\n");

    // Fork to handle input and output in separate processes
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // Child process for reading from server
        while (1) {
            ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received == -1) {
                if (errno == EINTR) {
                    continue;  // Retry if interrupted by signal
                }
                perror("recv");
                break;
            }
            if (bytes_received == 0) {
                printf("Server disconnected\n");
                break;
            }
            buffer[bytes_received] = '\0';
            printf("Server (%zd bytes): %s\n", bytes_received, buffer);
            total_bytes_received += bytes_received;
        }
        printf("Total bytes received: %zd\n", total_bytes_received);
        close(client_fd);
        exit(EXIT_SUCCESS);
    } else {  // Parent process for sending input
        while (1) {
            printf("You: ");
            if (fgets(message, sizeof(message), stdin) == NULL) {
                if (feof(stdin)) {
                    printf("EOF detected. Exiting...\n");
                    break;
                }
                perror("fgets");
                break;
            }
            message[strcspn(message, "\n")] = '\0';  // Remove newline character

            ssize_t bytes_sent = send(client_fd, message, strlen(message), 0);
            if (bytes_sent == -1) {
                perror("send");
                break;
            }
            printf("Sent (%zd bytes): %s\n", bytes_sent, message);
            total_bytes_sent += bytes_sent;
        }

        // Wait for the child process to finish
        close(client_fd);
        wait(NULL);
        printf("Total bytes sent: %zd\n", total_bytes_sent);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *host = argv[1];
    int port = atoi(argv[2]);

    start_client(host, port);

    return 0;
}
