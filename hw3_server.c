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
    int serv_sock, clnt_sock;  // Server and client socket descriptors
    struct sockaddr_in serv_addr, clnt_addr;  // Server and client address structures
    socklen_t clnt_addr_size;  // Size of the client address structure
    char file_name[BUF_SIZE];  // Buffer to hold the requested file name

    int file_fd;  // File descriptor for the requested file
    Packet packet;  // Packet structure instance
    int seq = SEQ_START;  // Initialize sequence number

    // Check if the correct number of arguments is provided
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Create a socket
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    // Initialize server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Bind the socket to the server address
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    // Listen for incoming connections
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    // Accept a client connection
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");

    // Display server information
    printf("-----------------------------------------\n");
    printf("           File Transmission Server      \n");
    printf("-----------------------------------------\n");

    // Receive the requested file name from the client
    read(clnt_sock, &packet, sizeof(Packet));
    strcpy(file_name, packet.buf);
    printf("Requested file: %s\n", packet.buf);

    // Open the requested file
    file_fd = open(packet.buf, O_RDONLY);
    if (file_fd == -1) {
        // If the file is not found, send an error message to the client
        strcpy(packet.buf, "File Not Found");
        write(clnt_sock, &packet, sizeof(Packet));
        printf("File Not Found\n");
        close(clnt_sock);
        close(serv_sock);
        return 0;
    }

    // File exists, start transmission
    printf("[Server] sending %s\n", file_name);
    while ((packet.buf_len = read(file_fd, packet.buf, BUF_SIZE)) > 0) {
        packet.seq = seq;  // Set the sequence number
        write(clnt_sock, &packet, sizeof(Packet));  // Send the packet to the client
        read(clnt_sock, &packet, sizeof(Packet));  // Receive acknowledgment from the client

        // Print sequence and acknowledgment numbers
        printf("[Server] Tx SEQ: %d, %d byte data\n", seq, packet.buf_len);
        printf("[Server] Rx ACK: %d\n", packet.ack);

        printf("\n");
        // Clear buffer to avoid accidental printing of residual data
        memset(packet.buf, 0, BUF_SIZE);

        seq += packet.buf_len;  // Increment sequence number
    }

    // Transmission complete
    printf("%s sent (%d Bytes)\n", file_name, seq);
    printf("Server Socket close\n");
    close(file_fd);  // Close the file descriptor
    close(clnt_sock);  // Close the client socket
    close(serv_sock);  // Close the server socket
    return 0;
}

// Function to handle errors by printing a message and exiting the program
void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}