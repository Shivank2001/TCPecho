#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>

#define BUFFER_SIZE 1024

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t total_bytes_received = 0;
    ssize_t total_bytes_sent = 0;

    printf("Child process ID: %d\n", getpid());

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
            printf("Client disconnected\n");
            break;
        }

        buffer[bytes_received] = '\0';
        // Remove newline character if present
        buffer[strcspn(buffer, "\n")] = '\0';
        printf("Received (%zd bytes): %s\n", bytes_received, buffer);
        total_bytes_received += bytes_received;

        // Echo the data back to the client
        ssize_t bytes_sent = send(client_fd, buffer, bytes_received, 0);
        if (bytes_sent == -1) {
            perror("send");
            break;
        }
        printf("Sent (%zd bytes): %s\n", bytes_sent, buffer);
        total_bytes_sent += bytes_sent;
    }

    printf("Total bytes received: %zd\n", total_bytes_received);
    printf("Total bytes sent: %zd\n", total_bytes_sent);

    close(client_fd);
}

void start_server(const char *host, int port) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(host);
    server_addr.sin_port = htons(port);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s:%d\n", host, port);
    printf("Parent process ID: %d\n", getpid());

    while (1) {
        // Accept incoming connection
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf("Client connected\n");

        // Fork to handle each client in a new process
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(client_fd);
            continue;
        }

        if (pid == 0) {  // Child process
            close(server_fd);  // Close the server socket in the child process
            printf("Forked child process ID: %d (Parent ID: %d)\n", getpid(), getppid());
            handle_client(client_fd);
            exit(EXIT_SUCCESS);
        } else {  // Parent process
            close(client_fd);  // Close the client socket in the parent process
        }
    }

    // Close server socket
    close(server_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *host = argv[1];
    int port = atoi(argv[2]);

    start_server(host, port);

    return 0;
}
