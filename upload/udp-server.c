#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <IP> <port>\n", argv[0]);
        return 1;
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        fprintf(stderr, "invalid IP address\n");
        return 1;
    }
    int servfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (servfd < 0) {
        perror("cannot create socket");
        return 1;
    }
    if (bind(servfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("cannot bind");
        return 1;
    }
    while (1) {
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        char buf[1024];
        int ret = recvfrom(servfd, buf, sizeof(buf), 0, (struct sockaddr*)&clientaddr, &clientaddrlen);
        if (ret < 0) {
            continue;
        }
        int result = 0;
        int i;
        for (i = 0; i < ret; ++i) {
            result += buf[i];
        }
    }
    return 0;
}
