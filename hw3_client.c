// 2022427833 - Lath Nishtha
//TCP file transfer 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define BUF_SIZE 1024  // Buffer size for file data
#define SEQ_START 0    // Initial sequence number

// Packet structure to hold sequence number, acknowledgment number, buffer length, and data buffer
typedef struct {
    int seq;
    int ack;
    int buf_len;
    char buf[BUF_SIZE];
} Packet;

// Function to handle errors by printing a message and exiting the program
void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;  // Socket descriptor
    struct sockaddr_in serv_addr;  // Server address structure
    Packet packet;  // Packet structure instance
    int seq = SEQ_START;  // Initialize sequence number
    char file_name[BUF_SIZE];  // Buffer to hold the requested file name

    // Check if the correct number of arguments is provided
    if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // Create a socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    // Initialize server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    // Get the file name from user input
    printf("Input file name: ");
    scanf("%s", packet.buf);
    strcpy(file_name, packet.buf);
    write(sock, &packet, sizeof(Packet));  // Send the file name to the server

    // Receive the file
    while (read(sock, &packet, sizeof(Packet)) > 0) {
        // Check if the file is not found
        if (strcmp(packet.buf, "File Not Found") == 0) {
            printf("File Not Found\n");
            break;
        }

        // Print sequence and acknowledgment numbers
        printf("[Client] Rx SEQ: %d, len: %d bytes\n", packet.seq, packet.buf_len);
        printf("[Client] Tx ACK: %d\n", packet.seq + packet.buf_len);

        printf("\n");
        packet.ack = packet.seq + packet.buf_len;  // Send acknowledgment
        write(sock, &packet, sizeof(Packet));

        // Clear buffer to avoid accidental printing of residual data
        memset(packet.buf, 0, BUF_SIZE);

        seq += packet.buf_len;  // Increment sequence number
    }

    // Transmission complete
    printf("%s received (%d Bytes)\n", file_name, seq);
    printf("Client Socket close\n");
    close(sock);  // Close the socket
    return 0;
}

// Function to handle errors by printing a message and exiting the program
void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

