#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

const int BUFSIZE = 1024;

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return -1;
    }
    short port = atoi(argv[1]);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("cannot create socket");
        return -1;
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("cannot bind");
        return -1;
    }

    while (1) {
        printf("recieving...\n");

        char buf[BUFSIZE];
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        int count = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&clientaddr, &clientaddrlen);
        if (count < 0) {
            perror("cannot recvfrom");
            continue;
        }
        char clientaddrstr[128];
        inet_ntop(AF_INET, &clientaddr.sin_addr, clientaddrstr, 128);
        buf[count] = 0;
        printf("recieved %d bytes from %s:%d: %s\n", count, clientaddrstr, ntohs(clientaddr.sin_port), buf);
        strcat(buf, " from server");

        if (sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&clientaddr, clientaddrlen) < 0) {
            perror("cannot send");
            continue;
        }
    }

    return 0;
}



