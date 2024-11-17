#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BOARD_SIZE 5
#define BUFFER_SIZE 1024
#define GAME_REQ 0
#define GAME_RES 1
#define GAME_END 2

typedef struct {
    int cmd;
    char ch;
} REQ_PACKET;

typedef struct {
    int cmd;
    char board[BOARD_SIZE][BOARD_SIZE];
    int result;
} RES_PACKET;

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    socklen_t addr_size;
    char guess;

    if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // Create a UDP socket
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    // Initialize server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    addr_size = sizeof(serv_addr);

    while (1) {
        // Get user's guessed letter
        printf("Enter an uppercase letter to guess (or 'q' to quit): ");
        scanf(" %c", &guess);
        
        if (guess == 'q') {
            printf("Exiting the game.\n");
            break;
        }

        // Prepare request packet
        REQ_PACKET req_packet;
        req_packet.cmd = GAME_REQ;
        req_packet.ch = guess;

        // Send guess to server
        sendto(sock, &req_packet, sizeof(req_packet), 0, (struct sockaddr*)&serv_addr, addr_size);

        // Receive response from server
        RES_PACKET res_packet;
        recvfrom(sock, &res_packet, sizeof(res_packet), 0, (struct sockaddr*)&serv_addr, &addr_size);

        // Display the board and guessed count
        printf("Current Board:\n");
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                printf("%c ", res_packet.board[i][j]);
            }
            printf("\n");
        }

        printf("Correct guesses so far: %d\n", res_packet.result);

        // Check for game end condition
        if (res_packet.cmd == GAME_END) {
            printf("Game Over! All letters guessed.\n");
            break;
        }
    }

    close(sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
