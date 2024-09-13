#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define MAX 80
#define SA struct sockaddr

void handle_sigchld(int sig) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void func(int connfd) {
    char buff[MAX];
    for (;;) {
        memset(buff, 0, sizeof(buff));
        int bytes_read = read(connfd, buff, sizeof(buff) - 1);
        if (bytes_read < 0) {
            perror("Read error");
            break;
        } else if (bytes_read == 0) {
            printf("Client disconnected...\n");
            break;
        }
        printf("From client: %s", buff);
        
        // Echo the same message back to the client
        printf("Echoing back: %s", buff); // Visible echo message
        if (write(connfd, buff, bytes_read) < 0) {
            perror("Write error");
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    int sockfd, connfd;
    socklen_t len;
    struct sockaddr_in servaddr, client;
    int port;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number. Please specify a port between 1 and 65535.\n");
        return EXIT_FAILURE;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        perror("Socket bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 10) != 0) {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d..\n", port);
    len = sizeof(client);

    while (1) {
        connfd = accept(sockfd, (SA*)&client, &len);
        if (connfd < 0) {
            perror("Server accept failed");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(connfd);
            continue;
        }

        if (pid == 0) {
            close(sockfd);
            func(connfd);
            close(connfd);
            exit(EXIT_SUCCESS);
        } else {
            close(connfd);
        }
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
