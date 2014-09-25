#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

int threads;
int listenfd;
int epollfd;
const size_t EVENT_LEN = 10;
int activefds = 1;
int packets = 0;

void *workerFn(void *param);

int main(int argc, char **argv)
{
    if (argc != 5) {
        fprintf(stderr, "usage: %s <IP> <port> <threads> <listen backlog>\n", argv[0]);
        return 1;
    }
    // fill server address
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        fprintf(stderr, "IP address invalid\n");
        return 1;
    }
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("cannot create socket");
        return 1;
    }
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("cannot bind");
        return 1;
    }
    if (listen(listenfd, atoi(argv[4])) != 0) {
        perror("cannot listen");
        return 1;
    }
    // create epoll
    epollfd = epoll_create(1000);
    if (epollfd < 0) {
        perror("cannot create epoll fd");
        return 1;
    }
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLONESHOT;
    event.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event) == -1) {
        perror("epoll_ctl error");
        return 1;
    }

    threads = atoi(argv[3]);
    pthread_t tids[threads];
    int i;
    for (i = 0; i < threads; ++i) {
        if (pthread_create(&tids[i], NULL, workerFn, NULL) != 0) {
            perror("cannot create thread");
            return 1;
        }
    }
    int last = -1;
    int lastPacket = -1;
    while (1) {
        sleep(1);
        if (last != activefds || lastPacket != packets) {
            printf("active: %d packets: %d\n", activefds, packets - lastPacket);
        }
        last = activefds;
        lastPacket = packets;
    }
    //for (i = 0; i < threads; ++i) {
    //    pthread_join(tids[i], NULL);
    //}
    return 0;
}

void add_new_fd(int fd)
{
    //printf("new %d %d\n", fd, pthread_self());
    __sync_fetch_and_add(&activefds, 1);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLONESHOT;
    event.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) == -1) {
        perror("epoll_ctl error");
        exit(1);
    }
}

void reenable_fd(int fd)
{
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLONESHOT;
    event.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) == -1) {
        perror("epoll_ctl error");
        exit(1);
    }
}

void close_fd(int fd)
{
    __sync_fetch_and_add(&activefds, -1);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

void *workerFn(void *param)
{
    while (1) {
        struct epoll_event events[EVENT_LEN];
        int n = epoll_wait(epollfd, events, EVENT_LEN, 10000);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("epoll_wait error");
            exit(1);
        }
        int i;
        int reenable = 1;
        for (i = 0; i < n; ++i) {
            if (events[i].data.fd == listenfd) {
                int fd = accept(listenfd, NULL, NULL);
                if (fd > 0) {
                    add_new_fd(fd);
                } else {
                    perror("accept error");
                    exit(1);
                }
            } else {
                char buf[1024];
                while (1) {
                    ssize_t n = recv(events[i].data.fd, buf, sizeof(buf), MSG_DONTWAIT); 
                    if (n == 0) {
                        reenable = 0;
                        close_fd(events[i].data.fd);
                        break;
                    } else if (n < 0) {
                        if (errno == EINTR) {
                            continue;
                        } else if (errno == EAGAIN) {
                            break;
                        } else {
                            reenable = 0;
                            close_fd(events[i].data.fd);
                            break;
                        }
                    } else {
                        __sync_fetch_and_add(&packets, 1);
                        int pos = 0;
                        while (pos < n) {
                            int sent = send(events[i].data.fd, buf+pos, n-pos, 0);
                            if (sent <= 0) {
                                break;
                            }
                            pos += sent;
                        }
                        if (pos < n) {
                            reenable = 0;
                            close_fd(events[i].data.fd);
                        }
                    }
                    if (n < sizeof(buf)) {
                        break;
                    }
                }
            }

            if (reenable) {
                reenable_fd(events[i].data.fd);
            }
        }
    }
}

