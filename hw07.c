// 2022427833 - Lath Nishtha

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUF_SIZE 120
#define TTL 64

// Function to handle errors by printing a message and exiting the program
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
    char message[BUF_SIZE];
    snprintf(message, BUF_SIZE, "[%s] ", argv[3]); // Format the name for sending messages

    // Create receiving socket
    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (recv_sock == -1) {
        error_handling("recv_sock socket() error");
    }

    // Allow multiple sockets to use the same PORT number
    int reuse = 1;
    if (setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&reuse, sizeof(reuse)) == -1)
        error_handling("recv_sock setsockopt() error (SO_REUSEADDR)");

    // Enable multicast loopback on recv_sock
    int loop = 1;
    if (setsockopt(recv_sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
        error_handling("recv_sock setsockopt() error (IP_MULTICAST_LOOP)");

    // Initialize the address structure for receiving
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to all interfaces
    recv_addr.sin_port = htons(atoi(argv[2]));

    // Bind the receiving socket to the address
    if (bind(recv_sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) == -1)
        error_handling("recv_sock bind() error");

    // Join the multicast group
    join_addr.imr_multiaddr.s_addr = inet_addr(argv[1]); // Multicast group IP
    join_addr.imr_interface.s_addr = htonl(INADDR_ANY);  // Use all available interfaces

    if (setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_addr, sizeof(join_addr)) == -1)
        error_handling("recv_sock setsockopt() error (IP_ADD_MEMBERSHIP)");

    // Create sending socket
    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (send_sock == -1) {
        error_handling("send_sock socket() error");
    }

    // Allow multiple sockets to use the same PORT number
    int send_reuse = 1;
    if (setsockopt(send_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&send_reuse, sizeof(send_reuse)) == -1)
        error_handling("send_sock setsockopt() error (SO_REUSEADDR)");

    // Initialize the address structure for sending
    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = inet_addr(argv[1]); // Multicast group IP
    send_addr.sin_port = htons(atoi(argv[2]));

    // Set the TTL (Time to Live) for the multicast packets
    int ttl = TTL;
    if (setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&ttl, sizeof(ttl)) == -1)
        error_handling("send_sock setsockopt() error (IP_MULTICAST_TTL)");

    // Enable multicast loopback on send_sock
    if (setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
        error_handling("send_sock setsockopt() error (IP_MULTICAST_LOOP)");

    // Fork the process to create a child process
    pid = fork();
    if (pid == -1)
        error_handling("fork() error");

    if (pid == 0) { // Child Process: Receiver
        while (1) {
            int str_len = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0, NULL, 0);
            if (str_len < 0) {
                perror("recvfrom() error");
                break;
            }
            buf[str_len] = 0; // Null-terminate the received string
            printf("%s", buf);
            fflush(stdout); // Ensure immediate output
        }
        close(recv_sock);
    } else { // Parent Process: Sender
        while (1) {
            fflush(stdout);
            if (fgets(buf, BUF_SIZE - strlen(message), stdin) == NULL) {
                perror("fgets() error");
                break;
            }
            if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")) { // Check for quit command
                kill(pid, SIGKILL); // Kill the child process
                printf("Exiting...\n");
                break;
            }
            snprintf(message + strlen(argv[3]) + 2, BUF_SIZE - strlen(message), "%s", buf); // Prepend the name to the message
            if (sendto(send_sock, message, strlen(message), 0, (struct sockaddr*)&send_addr, sizeof(send_addr)) == -1)
                perror("sendto() error");
        }
        close(send_sock);
    }
    return 0;
}