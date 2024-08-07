#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
// #include <pthread.h>
#include <sys/epoll.h>
#include <sys/select.h>

#define SERV_PORT 9981

int main(int argc, char *argv[]) {
    char buf[BUFSIZ], clt_ip[1024];
    int lfd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);
    bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(lfd, 128);
    struct sockaddr_in c_addr;
    socklen_t c_addr_len = sizeof(c_addr);
    
    int ep_root = epoll_create(1024);
    struct epoll_event tmp_event, events[BUFSIZ];
    tmp_event.events = EPOLLIN;
    tmp_event.data.fd = lfd;
    epoll_ctl(ep_root, EPOLL_CTL_ADD, lfd, &tmp_event);
    while (1) {
        int nready = epoll_wait(ep_root, events, BUFSIZ, -1);
        for (int i = 0; i < nready; i++) {
            if (!(events[i].events & EPOLLIN)) {
                continue;
            }
            int sock_fd = events[i].data.fd;
            if (sock_fd == lfd) {
                int cfd = accept(lfd, (struct sockaddr*)&c_addr, &c_addr_len);
                printf("client ip: %s, port: %d\n", 
                    inet_ntop(AF_INET, (void*)&(c_addr.sin_addr.s_addr), clt_ip, 1024), ntohs(c_addr.sin_port));
                tmp_event.data.fd = cfd;
                epoll_ctl(ep_root, EPOLL_CTL_ADD, cfd, &tmp_event);
            } else {
                int read_len = read(sock_fd, buf, BUFSIZ);
                if (read_len == -1) {
                    perror("read error");
                    exit(1);
                } else if (read_len == 0) {
                    epoll_ctl(ep_root, EPOLL_CTL_DEL, sock_fd, NULL);
                    close(sock_fd);
                } else {
                    for (int j = 0; j < read_len; j++) {
                        buf[j] = toupper(buf[j]);
                    }
                    write(sock_fd, buf, read_len);
                    write(STDOUT_FILENO, buf, read_len);
                }
            }
        }
    }
    return 0;
}