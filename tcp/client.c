#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

struct sockaddr_in servaddr;
socklen_t socklen;
void *client(void *param);
int per;

int main(int argc, char **argv)
{
    if (argc != 5) {
        fprintf(stderr, "usage: %s <IP> <port> <threads> <per>\n", argv[0]);
        return 1;
    }
    per = atoi(argv[4]);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        fprintf(stderr, "<IP> address invalid\n");
        return 1;
    }
    servaddr.sin_port = htons(atoi(argv[2]));
    socklen = sizeof(servaddr);

    int threads = atoi(argv[3]);
    pthread_t tids[threads];
    int i;
    for (i = 0; i < threads; ++i) {
        if (pthread_create(&tids[i], NULL, client, NULL) != 0) {
            perror("pthread_create failed");
            exit(1);
        }
    }

    for (i = 0; i < threads; ++i) {
        pthread_join(tids[i], NULL);
    }

    return 0;
}

void *client(void *param)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(fd, (struct sockaddr*)&servaddr, socklen) < 0) {
        perror("cannot connect");
        close(fd);
        return NULL;
    }

    char buf[128] = "hello, world";
    if (send(fd, buf, strlen(buf), 0) < 0) {
        perror("cannot send");
        close(fd);
        return NULL;
    }
    if (recv(fd, buf, sizeof(buf), 0) < 0) {
        perror("cannot recv");
        close(fd);
        return NULL;
    }

    close(fd);
    return NULL;
}

