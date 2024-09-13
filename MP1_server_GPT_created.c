#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

void error_handling(char *message) {
    perror(message);
    exit(1);
}

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    pid_t pid;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1) {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
            error_handling("accept() error");

        pid = fork(); // Fork a new process
        if (pid == -1) {
            close(clnt_sock);
            continue;
        }

        if (pid == 0) { // Child process
            close(serv_sock);
            char message[1024];
            int str_len;
            
            while ((str_len = read(clnt_sock, message, 1024)) != 0) {
                write(clnt_sock, message, str_len); // Echo message back
            }

            close(clnt_sock);
            puts("Client disconnected.");
            return 0;
        } else { // Parent process
            close(clnt_sock);
        }
    }
    close(serv_sock);
    return 0;
}
