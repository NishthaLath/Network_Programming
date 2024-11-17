// Lath Nishtha, 2022427833
// Assignment 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_SIZE 100
#define TIME_REQ 0
#define TIME_RES 1

typedef struct {
    int cmd; // TIME_REQ or TIME_RES
    char time_msg[BUF_SIZE];
} PACKET;

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    PACKET packet;
    time_t t;
    struct tm *p;
    
    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if(clnt_sock == -1)
        error_handling("accept() error");

    printf("Connected client, client_sock: %d\n", clnt_sock);

    while(1) {
        int str_len = read(clnt_sock, &packet, sizeof(packet));
        if(str_len == 0) break;

        if(packet.cmd == TIME_REQ) {
            printf("[Server] Rx TIME_REQ\n");

            time(&t);
            p = localtime(&t);
            sprintf(packet.time_msg, "%d-%d-%d %d:%d:%d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
            packet.cmd = TIME_RES;

            printf("[Server] Tx time: %s\n", packet.time_msg);
            write(clnt_sock, &packet, sizeof(packet));
        }
    }

    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
