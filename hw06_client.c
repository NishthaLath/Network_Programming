#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>

#define PORT 9190
#define BUFFER_SIZE 2048

void sender_mode(int sock_fd);
void receiver_mode(int sock_fd);

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;
    char mode;

    // Initialize socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Choose mode (1: Sender, 2: Receiver): ");
    scanf(" %c", &mode);

    if (mode == '1') {
        sender_mode(sock_fd);
    } else if (mode == '2') {
        receiver_mode(sock_fd);
    } else {
        printf("Invalid mode selected.\n");
    }

    close(sock_fd);
    return 0;
}

void sender_mode(int sock_fd) {
    int file_fd = open("rfc1180.txt", O_RDONLY);
    char buffer[BUFFER_SIZE];
    fd_set read_fds;
    struct timeval timeout;

    if (file_fd < 0) {
        perror("File open failed");
        return;
    }

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sock_fd, &read_fds);

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(file_fd, buffer, BUFFER_SIZE);

        if (bytes_read > 0) {
            send(sock_fd, buffer, bytes_read, 0);
            sleep(1);
        } else {
            break;
        }

        if (select(sock_fd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(sock_fd, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0) {
                printf("Received: %s", buffer);
            }
        }
    }

    close(file_fd);
}

void receiver_mode(int sock_fd) {
    char buffer[BUFFER_SIZE];
    fd_set read_fds;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sock_fd, &read_fds);

        if (select(sock_fd + 1, &read_fds, NULL, NULL, NULL) > 0) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(sock_fd, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0) {
                printf("Echoing back: %s", buffer);
                send(sock_fd, buffer, bytes_received, 0);
            }
        }
    }
}
