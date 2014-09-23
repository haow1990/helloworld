#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    if (argc != 5) {
        fprintf(stderr, "usage: %s <IP> <port> <MBs> <bytes per>\n", argv[0]);
        return 1;
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = ntohs(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        fprintf(stderr, "invalid IP address\n");
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("cannot create socket");
    }
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("cannot connect");
        return 1;
    }
    int total = atoi(argv[3]) << 20;
    int unit = atoi(argv[4]);
    char buf[unit];
    int i;
    for (i = 0; i < unit; ++i) {
        buf[i] = 1;
    }
    int current;
    for (current = 0; current < total;) {
        int ret = send(sockfd, buf, unit, 0);
        if (ret < 0) {
            break;
        }
        current += ret;
    } 
    close(sockfd);
    printf("sent %d bytes\n", current);

    return 0;
}

