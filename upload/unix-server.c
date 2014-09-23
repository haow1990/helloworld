#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <socket file path>\n", argv[0]);
        return 1;
    }
    struct sockaddr_un servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, argv[1]);

    int servfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (servfd < 0) {
        perror("cannot create socket");
        return 1;
    }
    if (bind(servfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("cannot bind");
        return 1;
    }
    if (listen(servfd, 10) != 0) {
        perror("cannot listen");
        return 1;
    }

    printf("accepting...\n");
    while (1) {
        struct sockaddr_un clientaddr;
        bzero(&clientaddr, sizeof(clientaddr));
        socklen_t clientaddrlen = sizeof(clientaddr);
        int sockfd = accept(servfd, (struct sockaddr*)&clientaddr, &clientaddrlen);
        if (sockfd < 0) {
            continue;
        }
        printf("accepted connection from %s\n", clientaddr.sun_path);
        char buf[1024];
        int result = 0;
        int packets = 0;
        while (1) {
            ++packets;
            int ret = read(sockfd, buf, sizeof(buf), 0);
            if (ret <= 0) {
                break;
            }
            int i;
            for (i = 0; i < ret; ++i) {
                result += buf[i];
            }
        }
        close(sockfd);
        printf("closed connection %d %d\n", result, packets);
    }
    return 0;
}
