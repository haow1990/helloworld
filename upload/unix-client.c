#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char **argv)
{
    printf("running unix client\n");
    if (argc != 4) {
        fprintf(stderr, "usage: %s <path> <MBs> <bytes per>\n", argv[0]);
        return 1;
    }
    struct sockaddr_un servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, argv[1]);

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("cannot create socket");
    }
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("cannot connect");
        return 1;
    }
    int total = atoi(argv[2]) << 20;
    int unit = atoi(argv[3]);
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


