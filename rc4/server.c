#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

// Usage: ./server [PORT] [ENCRYPTION KEY]

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);


typedef struct PASSING_PARAMS
{
    char *password;
    int len;
    int in;
    int out;
} PassingParams;

void swap(int *a,int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void *encrypt(void *ptr)
{
    PassingParams *paramPtr = (PassingParams *)ptr;
    
    int S[256];
    int j = 0;
    int i = 0;
    int k = 0;
    int rnd = 0;
    for (i = 0; i < 256; i++)
    {
        S[i] = i;
    }
    for (i = 0; i < 256; i++)
    {
        j = (j + S[i] + paramPtr->password[i % paramPtr->len]) % 256;
        swap(&S[i], &S[j]);
    }
    i = 0;
    j = 0;

    while (1)
    {
        char bufA[1];
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        swap(&S[i], &S[j]);
        rnd = S[(S[i] + S[j]) % 256];
        read(paramPtr->in, bufA, sizeof(bufA));
        bufA[0] = (char)rnd ^ bufA[0];
        write(paramPtr->out, bufA, sizeof(bufA));
    }
}



int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: ./server [PORT] [ENCRYPTION KEY]\n");
        exit(1);
    }
    int sock, socket_fd, client_len;
    struct sockaddr_in server, client;

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
        close(sock);
        exit(2);
    }
    listen(sock, 0);
    fprintf(stderr, "%s", "Listening\n");
    client_len = sizeof(client);
    socket_fd = accept(sock, (struct sockaddr *)&client, &client_len);
    if (socket_fd < 0)
    {
        close(sock);
        close(socket_fd);
        exit(3);
    }
    fprintf(stderr, "%s", "Shell Connected!\n");
    close(sock);

    pthread_t reader;
    PassingParams *socket_to_shell;
    socket_to_shell = (PassingParams *)malloc(sizeof(PassingParams));
    socket_to_shell->password = argv[2];
    socket_to_shell->len = strlen(argv[2]);
    socket_to_shell->in = socket_fd;
    socket_to_shell->out = 0;
    pthread_create(&reader, NULL, &encrypt, (void *)socket_to_shell);

    pthread_t writer;
    PassingParams *shell_to_socket;
    shell_to_socket = (PassingParams *)malloc(sizeof(PassingParams));
    shell_to_socket->password = argv[2];
    shell_to_socket->len = strlen(argv[2]);
    shell_to_socket->in = 1;
    shell_to_socket->out = socket_fd;
    pthread_create(&writer, NULL, &encrypt, (void *)shell_to_socket);

    while (1)
    {
        sleep(60);
        // shell is running
    }

    close(socket_fd);
    exit(0);
    return 0;
}