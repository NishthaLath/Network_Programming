#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 9190
#define BUFFER_SIZE 2048

int main() {
    int server_fd, client1_fd, client2_fd;
    struct sockaddr_in server_addr;
    fd_set read_fds, temp_fds;
    char buffer[BUFFER_SIZE];
    int max_fd;

    // Initialize server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    client1_fd = accept(server_fd, NULL, NULL);
    if (client1_fd < 0) {
        perror("Accept failed for Client #1");
        exit(EXIT_FAILURE);
    }
    printf("Client Connected : %d (fd_max: %d)\n", client1_fd, client1_fd);

    client2_fd = accept(server_fd, NULL, NULL);
    if (client2_fd < 0) {
        perror("Accept failed for Client #2");
        exit(EXIT_FAILURE);
    }
    printf("Client Connected : %d (fd_max: %d)\n", client2_fd, client2_fd);

    FD_ZERO(&read_fds);
    FD_SET(client1_fd, &read_fds);
    FD_SET(client2_fd, &read_fds);
    max_fd = (client1_fd > client2_fd) ? client1_fd : client2_fd;

    while (1) {
        temp_fds = read_fds;

        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        // Check Client #1
        if (FD_ISSET(client1_fd, &temp_fds)) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_read = read(client1_fd, buffer, BUFFER_SIZE);
            if (bytes_read > 0) {
                printf("Forward [%d] ---> [%d]\n", client1_fd, client2_fd);
                send(client2_fd, buffer, bytes_read, 0);
            } else {
                break;
            }
        }

        // Check Client #2
        if (FD_ISSET(client2_fd, &temp_fds)) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_read = read(client2_fd, buffer, BUFFER_SIZE);
            if (bytes_read > 0) {
                printf("Backward [%d] ---> [%d]\n", client2_fd, client1_fd);
                send(client1_fd, buffer, bytes_read, 0);
            } else {
                break;
            }
        }
    }

    close(client1_fd);
    close(client2_fd);
    close(server_fd);
    return 0;
}
