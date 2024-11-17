// 2022427833 Lath Nishtha
//Tcp Numbers Guessing Game

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 9190
#define BOARD_SIZE 5
#define GAME_REQ 1
#define GAME_RES 2
#define GAME_END 3
#define GAME_END_ACK 4

typedef struct {
    int cmd;
    int num;
} REQ_PACKET;

typedef struct {
    int cmd;
    int num;
    int game_board[BOARD_SIZE][BOARD_SIZE];
    int count;
} RES_PACKET;

void display_guessed_board(int guessed_board[BOARD_SIZE][BOARD_SIZE]) {
    printf("Client Guessed Board:\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("+----+----+----+----+----+\n");
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (guessed_board[i][j] == 0) {
                printf("|    ");
            } else {
                printf("| %2d ", guessed_board[i][j]);
            }
        }
        printf("|\n");
    }
    printf("+----+----+----+----+----+\n");
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    REQ_PACKET req;
    RES_PACKET res;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connect failed");
        close(sock);
        exit(1);
    }

    srand(time(0));

    printf("---------------------------------\n");
    printf("           Start Game Client           \n");
    printf("---------------------------------\n");

    while (1) {
        req.cmd = GAME_REQ;
        req.num = (rand() % 25) + 1;
        printf("[Client] Tx GAME_REQ(cmd: %d, num: %d)\n", req.cmd, req.num);
        send(sock, &req, sizeof(req), 0);

        recv(sock, &res, sizeof(res), 0);

        if (res.cmd == GAME_RES) {
            printf("[Client] Rx GAME_RES(cmd: %d, num: %d, count: %d)\n", res.cmd, res.num, res.count);
            display_guessed_board(res.game_board);
        } else if (res.cmd == GAME_END) {
            printf("[Client] Rx GAME_END(cmd: %d)\n", res.cmd);
            display_guessed_board(res.game_board);
            req.cmd = GAME_END_ACK;
            send(sock, &req, sizeof(req), 0);
            printf("[Client] Rx GAME_END_ACK(cmd: %d)\n", req.cmd);
            printf("Exit Client Program\n");
            break;
        }
    }

    close(sock);
    return 0;
}