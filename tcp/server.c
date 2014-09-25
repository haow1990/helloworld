#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

void *serve(void *param);

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    pid_t pid = getpid();
    printf("server running, pid is %d\n", (int)pid);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        fprintf(stderr, "IP Address invalid\n");
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("cannot bind");
        exit(1);
    }
    if (listen(sockfd, 1000) != 0) {
        perror("listen error");
        exit(1);
    }

    while (1) {
        struct sockaddr_in clientaddr;
        socklen_t socklen = sizeof(clientaddr);
        int fd = accept(sockfd, (struct sockaddr*)&clientaddr, &socklen);
        if (fd < 0) {
            perror("cannot accept");
            continue;
        }
        char buf[128];
        inet_ntop(AF_INET, &clientaddr.sin_addr, buf, sizeof(buf));
        printf("accepted connection from %s:%d\n", buf, ntohs(clientaddr.sin_port));
        pthread_t tid;
        if (pthread_create(&tid, NULL, serve, (void*)((long)fd)) != 0) {
            perror("cannot create thread");
            close(fd);
        }
        pthread_detach(tid);
    }
    return 0;
}


void *serve(void *param)
{
    int fd = (int)((long)param);
    char buf[1024];
    while (1) {
        int size = read(fd, buf, sizeof(buf) - 1, 0);
        if (size < 0) {
            perror("cannot read");
            break;
        } else if (size == 0) {
            //printf("closed\n");
            break;
        }
        buf[size] = 0;
        //printf("recieved %d bytes, strlen=%d: %s\n", size, strlen(buf), buf);
        //printf("\nsending back...\n");
        size = send(fd, buf, size, 0);
        if (size <= 0) {
            perror("cannot send");
            break;
        } else {
            //printf("sent %d bytes\n", size);
        }
    }
    close(fd);
    return NULL;
}

