#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

int scan(const char *ip, int port, int timeout_ms) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;

    fcntl(sock, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    int res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (res < 0 && errno != EINPROGRESS) {
        close(sock);
        return 0;
    }
    if (res == 0) {
        close(sock);
        return 1;
    }

    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET(sock, &write_set);

    struct timeval timeout = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    res = select(sock + 1, NULL, &write_set, NULL, &timeout);

    if (res > 0) {
        int err;
        socklen_t len = sizeof(err);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len);
        close(sock);
        return err == 0;
    }

    close(sock);
    return 0;
}

int main() {
    char ip[100];
    int start_port, end_port;
    int timeout = 200;

    printf("enter ip: ");
    scanf("%99s", ip);

    printf("start port (default 1): ");
    if (scanf("%d", &start_port) != 1) start_port = 1;

    printf("end port (default 1024): ");
    if (scanf("%d", &end_port) != 1) end_port = 1024;

    if (start_port < 1) start_port = 1;
    if (end_port > 65535) end_port = 65535;
    if (end_port < start_port) return 1;

    for (int port = start_port; port <= end_port; port++) {
        if (scan(ip, port, timeout)) {
            printf("port %d open\n", port);
        }
    }

    return 0;
}