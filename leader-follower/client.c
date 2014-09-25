#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

int threads;
int perconn;
struct sockaddr_in servaddr;
int *count;

void *worker(void *param);
void print_count(int signum);

int main(int argc, char **argv)
{
    if (argc != 5) {
        fprintf(stderr, "usage: %s <IP> <port> <threads> <per>\n", argv[0]);
        exit(1);
    }
    threads = atoi(argv[3]);
    perconn = atoi(argv[4]);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        fprintf(stderr, "IP address invalid\n");
        exit(1);
    }
    count = (int*)malloc(sizeof(int) * threads);
    pthread_t tids[threads];
    int i;
    for (i = 0; i < threads; ++i) {
        count[i] = 0;
        if (pthread_create(&tids[i], NULL, worker, count+i) != 0) {
            perror("cannot create thread");
            exit(1);
        }
    }
    signal(SIGINT, print_count);
    int last = 0;
    while (1) {
        sleep(1);
        int current = 0;
        for (i = 0; i < threads; ++i) {
            current += count[i];
        }
        printf("total: %d; last second: %d\n", current, current - last);
        last = current;
    }
    //for (i = 0; i < threads; ++i) {
    //    pthread_join(tids[i], NULL);
    //}
    return 0;
}

int send_all(int fd, char *buf, int len)
{
    memset(buf, 8, len);
    int pos = 0;
    while (pos < len) {
        int ret = send(fd, buf + pos, len - pos, 0);
        if (ret < 0) {
            perror("cannot send");
            exit(1);
        }
        pos += ret;
    }
    return pos;
}

void *worker(void *param)
{
    int *count = (int*)param;
    int sockfds[perconn];
    int i;
    for (i = 0; i < perconn; ++i) {
        sockfds[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfds[i] < 0) {
            perror("cannot create socket");
            exit(1);
        }
        if (connect(sockfds[i], (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
            perror("cannot connect");
            exit(1);
        }
    }
    FILE *randfile = fopen("/dev/urandom", "r");
    char buf[128];
    while (1) {
        unsigned int idx;
        if (fread(&idx, sizeof(int), 1, randfile) < 1) {
            fprintf(stderr, "cannot read urandom\n");
            exit(1);
        }
        int fd = idx % perconn;
        send_all(sockfds[fd], buf, sizeof(buf));
        ++*count;
        recv(sockfds[fd], buf, sizeof(buf), O_NONBLOCK);
    }
    return NULL;
}

void print_count(int signum)
{
    int total = 0;
    int i;
    for (i = 0; i < threads; ++i) {
        total += count[i];
    }
    printf("total = %d\n", total);
    exit(0);
}


