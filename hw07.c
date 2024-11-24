// 2022427833 - Lath Nishtha

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 120
#define TTL 64

void error_handling(const char *message) {
    perror(message);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <GroupIP> <PORT> <Name>\n", argv[0]);
        exit(1);
    }

    int recv_sock, send_sock;
    struct sockaddr_in recv_addr, send_addr;
    struct ip_mreq join_addr;
    pid_t pid;
    char buf[BUF_SIZE];
    char name[BUF_SIZE];
    snprintf(name, BUF_SIZE, "[%s] ", argv[3]);

    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (recv_sock == -1) error_handling("socket() error");

    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    recv_addr.sin_port = htons(atoi(argv[2]));

    if (bind(recv_sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) == -1)
        error_handling("bind() error");

    join_addr.imr_multiaddr.s_addr = inet_addr(argv[1]);
    join_addr.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_addr, sizeof(join_addr)) == -1)
        error_handling("setsockopt() error (IP_ADD_MEMBERSHIP)");

    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (send_sock == -1) error_handling("socket() error");

    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = inet_addr(argv[1]);
    send_addr.sin_port = htons(atoi(argv[2]));

    int ttl = TTL;
    if (setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&ttl, sizeof(ttl)) == -1)
        error_handling("setsockopt() error (IP_MULTICAST_TTL)");

    pid = fork();
    if (pid == -1) error_handling("fork() error");

    if (pid == 0) { // Child Process: Receiver
        while (1) {
            int str_len = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0, NULL, 0);
            if (str_len < 0) break;
            buf[str_len] = 0;
            printf("%s\n", buf);
        }
        close(recv_sock);
    } else { // Parent Process: Sender
        while (1) {
            fgets(buf, BUF_SIZE - strlen(name), stdin);
            if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")) {
                kill(pid, SIGKILL);
                break;
            }
            snprintf(buf + strlen(name), BUF_SIZE - strlen(name), "%s", buf);
            sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&send_addr, sizeof(send_addr));
        }
        close(send_sock);
    }
    return 0;
}
