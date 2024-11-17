#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>

// Buffer size and initial sequence number
#define BUF_SIZE 1024
#define SEQ_START 0

// Packet commands
#define FILE_REQ 1
#define FILE_RES 2
#define FILE_END 3
#define FILE_NOT_FOUND 4
#define FILE_END_ACK 5

// Packet structure
typedef struct {
    int seq;          // Sequence number
    int ack;          // Acknowledgment number
    int buf_len;      // Number of bytes read or written
    char buf[BUF_SIZE];  // Buffer for file name or file content
} PACKET;

// Function to handle errors
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
    char fname[BUF_SIZE];
    int str_len;
    socklen_t addr_size;
    PACKET send_packet, recv_packet;
    struct sockaddr_in serv_addr;
    int total_rx_cnt = 0, total_rx_bytes = 0;

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // PACKET memory initialize
    memset(&recv_packet, 0, sizeof(PACKET));
    memset(&send_packet, 0, sizeof(PACKET));
    memset(fname, 0, sizeof(char) * BUF_SIZE);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");

    printf("Input file name: ");
    scanf("%s", fname);

    /*-----------------------------------------------------------
                  FILE_REQ 
          Client  ------->  Server
               (buf: file name)
    *-----------------------------------------------------------*/
    send_packet.seq = SEQ_START;
    send_packet.ack = 0;
    send_packet.buf_len = strlen(fname);
    strncpy(send_packet.buf, fname, send_packet.buf_len);

    write(sock, (void*)&send_packet, sizeof(PACKET));

    printf("[Tx] seq: %d, file name: %s\n", send_packet.seq, send_packet.buf);

    while (1) {

        read(sock, (void*)&recv_packet, sizeof(PACKET));

        if (recv_packet.seq == FILE_RES) {
            total_rx_cnt++;
            total_rx_bytes += recv_packet.buf_len;
            recv_packet.buf[recv_packet.buf_len] = '\0';    // added by csjung
            printf("%s", recv_packet.buf);
        }
        else if (recv_packet.seq == FILE_NOT_FOUND) {
            printf("[Rx] seq: %d, %s: File Not Found\n", recv_packet.seq, fname);
            break;

        }
        else if (recv_packet.seq == FILE_END) {
            total_rx_cnt++;
            total_rx_bytes += recv_packet.buf_len;
            recv_packet.buf[recv_packet.buf_len] = '\0';    // added by csjung
            printf("%s\n", recv_packet.buf);

            printf("---------------------------\n");
            printf("[Rx] seq: %d, FILE_END \n", recv_packet.seq);
            /*-----------------------------------------------------------
                      FILE_END_ACK 
              Client  ------->  Server
            *-----------------------------------------------------------*/
            send_packet.seq = FILE_END_ACK;
            write(sock, (void*)&send_packet, sizeof(PACKET));
            printf("[Tx] seq: %d, FILE_END_ACK\n", send_packet.seq);
            break;
        }
        else {
            printf("[Rx] Invalid seq: %d\n", recv_packet.seq);
        }

        memset(&recv_packet, 0, sizeof(PACKET));
        memset(&send_packet, 0, sizeof(PACKET));

    }
    printf("------------------------------------\n");
    printf("Total Rx count: %d, bytes: %d\n", total_rx_cnt, total_rx_bytes);
    printf("TCP Client Socket Close!\n");
    printf("------------------------------------\n");
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}