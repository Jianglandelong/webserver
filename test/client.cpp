#include <arpa/inet.h>
// #include <linux/in.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void syserr(char *str) {
    perror(str);
    exit(1);
}

const uint16_t SERVER_PORT = 9981;

int main(int argc, char *argv[]) {
    int cfd;
    char buf[BUFSIZ], s_buf[BUFSIZ];
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1) {
        syserr("socket error");
    }
    struct sockaddr_in s_addr;
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_port = htons(SERVER_PORT);
    int ret = connect(cfd, (struct sockaddr*)&s_addr, sizeof(s_addr));
    if (ret == -1) {
        syserr("connect error");
    }
    ssize_t len = 0;
    do {
        len = read(STDIN_FILENO, buf, BUFSIZ);
        if (len == -1) {
            syserr("read stdin error");
        } else if (len == 0) {
            write(cfd, buf, 0);
            break;
        }
        len = write(cfd, buf, len);
        if (len == -1) {
            syserr("write to server error");
        }
        printf("send successfully\n");
        len = read(cfd, s_buf, BUFSIZ);
        if (len == -1) {
            syserr("read server error");
        }
        len = write(STDOUT_FILENO, s_buf, len);
        if (len == -1) {
            syserr("write stdout error");
        }
    } while (len > 0);
    printf("Write Over\n");
    close(cfd);
    return 0;
}