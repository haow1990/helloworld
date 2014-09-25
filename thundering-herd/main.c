#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>

int listenfd;
int threads;
int epollfd;

void *worker(void *param);

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s <IP> <port> <threads>\n", argv[0]);
        exit(1);
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        fprintf(stderr, "IP address invalid\n");
        exit(1);
    }
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("cannot create socket");
        exit(1);
    }
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    fcntl(listenfd, F_SETFL, O_NONBLOCK);
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("cannot bind");
        exit(1);
    }
    listen(listenfd, 10);

    epollfd = epoll_create(20);
    if (epollfd < 0) {
        perror("cannot create epoll fd");
        exit(1);
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) != 0) {
        perror("cannot add listenfd");
        exit(1);
    }

    threads = atoi(argv[3]);
    pthread_t tids[threads];
    int i;
    for (i = 0; i < threads; ++i) {
        if (pthread_create(&tids[i], NULL, worker, NULL) != 0) {
            perror("cannot create thread");
            exit(1);
        }
    }
    for (i = 0; i < threads; ++i) {
        pthread_join(tids[i], NULL);
    }
    return 0;
}

void *worker(void *param)
{
    while (1) {
        struct epoll_event events[20];
        int ret = epoll_wait(epollfd, events, 20, 1000);
        if (ret < 0) {
            perror("cannot epoll wait");
            exit(1);
        }
        if (ret == 0) {
            continue;
        }
        printf("waiting...\n");
        int fd = accept(listenfd, NULL, NULL);
        if (fd < 0) {
            if (errno == EAGAIN) {
                printf("accept EAGAIN\n");
                continue;
            }
            perror("cannot accept");
            exit(1);
        }
        close(fd);
    }
}

