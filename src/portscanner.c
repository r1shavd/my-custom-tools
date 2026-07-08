#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

// --- THESE TWO LINES FIX YOUR COMPILATION ERRORS ---
#include <sys/select.h>  // Defines fd_set, FD_ZERO, FD_SET, and select
#include <sys/time.h>    // Defines struct timeval

void scan_port(const char* ip, int port) {
    int sock;
    struct sockaddr_in target;
    fd_set fdset;
    struct timeval tv;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return;
    }

    // Fixed a small syntax typo from the previous snippet's nested fcntl call here
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr(ip);

    int res = connect(sock, (struct sockaddr*)&target, sizeof(target));

    if (res < 0) {
        if (errno == EINPROGRESS) {
            // Connection is in progress, wait for it
            FD_ZERO(&fdset);
            FD_SET(sock, &fdset);
            tv.tv_sec = 1; // 1 second timeout
            tv.tv_usec = 0;

            int select_res = select(sock + 1, NULL, &fdset, NULL, &tv);
            if (select_res == 1) {
                int so_error;
                socklen_t len = sizeof(so_error);
                getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
                
                if (so_error == 0) {
                    // If connection is found open, only then display

                    printf("Port %-5d OPEN\n", port);
                }
            } else if (select_res == 0) {
                printf("Port %-5d FILTERED", port);
            }
        } else {
            perror("Connect error");
        }
    } else if (res == 0) {
        printf("Port %-5d is OPEN\n", port);
    }

    close(sock);
}

int main() {
    const char* target_ip = "192.168.1.121";
    int start_port = 21;
    int end_port = 65400;

    printf("Starting scan on %s from port %d to %d...\n", target_ip, start_port, end_port);

    for (int port = start_port; port <= end_port; port++) {
        scan_port(target_ip, port);
    }

    printf("Scan complete.\n");
    return 0;
}