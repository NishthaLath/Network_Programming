#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

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
void initialize_board(char board[BOARD_SIZE][BOARD_SIZE]);
int check_and_update_board(char board[BOARD_SIZE][BOARD_SIZE], char guess, int *guessed_count);

int main(int argc, char *argv[]) {
    int serv_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    char board[BOARD_SIZE][BOARD_SIZE];
    int guessed_count = 0;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Create a UDP socket
    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
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

    initialize_board(board);

    while (1) {
        REQ_PACKET req_packet;
        RES_PACKET res_packet;
        memset(&req_packet, 0, sizeof(req_packet));
        memset(&res_packet, 0, sizeof(res_packet));

        // Receive packet from client
        clnt_addr_size = sizeof(clnt_addr);
        recvfrom(serv_sock, &req_packet, sizeof(req_packet), 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

        // if q is received, exit the game
        if (req_packet.ch == 'q') {
            printf("Exiting the game.\n");
            break;
        }

        printf("Received guess: %c\n", req_packet.ch);

        // Check guessed letter and update
        int found = check_and_update_board(board, req_packet.ch, &guessed_count);
        res_packet.cmd = (guessed_count == BOARD_SIZE * BOARD_SIZE) ? GAME_END : GAME_RES;
        memcpy(res_packet.board, board, sizeof(board));
        res_packet.result = guessed_count;

        // Display the board and guessed count
        printf("Current Board:\n");
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                printf("%c ", res_packet.board[i][j]);
            }
            printf("\n");
        }

        printf("Correct guesses so far: %d\n", res_packet.result);

        // Send response to client
        sendto(serv_sock, &res_packet, sizeof(res_packet), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);

        if (res_packet.cmd == GAME_END) {
            printf("Game ended. All letters guessed.\n");
            break;
        }
    }

    close(serv_sock);
    return 0;
}

void initialize_board(char board[BOARD_SIZE][BOARD_SIZE]) {
    srand(time(NULL));
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = 'A' + (rand() % 26);  // Random uppercase letter
        }
    }
}

int check_and_update_board(char board[BOARD_SIZE][BOARD_SIZE], char guess, int *guessed_count) {
    int found = 0;
    printf("   +-----+-----+-----+\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == guess) {
                board[i][j] = '*';  // Mark as guessed
                (*guessed_count)++;
                found = 1;
            }
        }
    }
    printf("   +-----+-----+-----+\n");
    return found;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
