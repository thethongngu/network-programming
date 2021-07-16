#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <sys/wait.h>

#include "npshell.h"

int main(int argc, char *argv[]) {

    struct addrinfo hints{};
    struct addrinfo *server;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status;
    if ((status = getaddrinfo(nullptr, argv[1], &hints, &server)) != 0) {
        fprintf(stderr, "Cannot getaddrinfo: %s\n", gai_strerror(status));
    }

    int server_fd;
    if ((server_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol)) < 0) {
        perror("Cannot create socket\n");
    }

    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) < 0) {
        perror("Error setsockopt");
    }

    if (bind(server_fd, server->ai_addr, server->ai_addrlen) < 0) {
        perror("Cannot bind socket\n");
    }
    freeaddrinfo(server);

    if (listen(server_fd, 35) < 0) {
        perror("Cannot listen on this socket\n");
    }

    struct sockaddr_storage client{};
    while (true) {
        socklen_t client_size = sizeof client;
        int client_fd = accept(server_fd, (struct sockaddr *) &client, &client_size);
        if (client_fd < 0) {
            perror("Cannot accept new connection");
            continue;
        }

        if (!fork()) {  // child process
            close(server_fd);
            dup2(client_fd, STDIN_FILENO);
            dup2(client_fd, STDOUT_FILENO);
            dup2(client_fd, STDERR_FILENO);
            NPShell shell;
            shell.run_interactive_shell();
            close(client_fd);
            exit(EXIT_SUCCESS);
        }

        close(client_fd);  // parent close client fd
        wait(nullptr);
    }
}
