#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

const int BUFSIZE = 1024;

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s <IP Address> <port>\n", argv[0]);
        return -1;
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        perror("cannot inet_pton");
        return -1;
    }
    servaddr.sin_family = AF_INET;
    short port = atoi(argv[2]);
    servaddr.sin_port = htons(port);
    char straddr[128];
    if (inet_ntop(AF_INET, &servaddr.sin_addr, straddr, sizeof(straddr)) == 0) {
        perror("cannot inet_ntop");
        return -1;
    }
    printf("remote address: %s:%d\n", straddr, ntohs(servaddr.sin_port));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("cannot create socket");
        return -1;
    }
    struct sockaddr_in clientaddr;
    bzero(&clientaddr, sizeof(clientaddr));
    if (bind(sockfd, (struct sockaddr*)&clientaddr, sizeof(clientaddr)) < 0) {
        perror("cannot bind");
        return 0;
    }
    if (inet_ntop(AF_INET, &clientaddr.sin_addr, straddr, sizeof(straddr)) == 0) {
        perror("cannot inet_ntop");
        return -1;
    }
    printf("local address: %s:%d\n", straddr, ntohs(clientaddr.sin_port));

    while (1) {
        char buf[BUFSIZE];
        scanf("%s", buf);
        int count = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        if (count < 0) {
            perror("cannot sendto");
            return -1;
        }
        count = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
        if (count < 0) {
            perror("cannot recvfrom");
            return -1;
        }
        buf[count] = 0;
        printf("recieved %d bytes from server: %s\n", count, buf);
    }
    return 0;
}

