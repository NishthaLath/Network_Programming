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

void fill_board(int board[BOARD_SIZE][BOARD_SIZE]) {
    srand(time(0));
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = (rand() % 25) + 1;
        }
    }
}

void display_board(int board[BOARD_SIZE][BOARD_SIZE], int guessed_board[BOARD_SIZE][BOARD_SIZE]) {
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("+----+----+----+----+----+\t+----+----+----+----+----+\n");
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 0) {
                printf("|    ");  
            } else {
                printf("| %2d ", board[i][j]);
            }
        }
        printf("|\t");
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (guessed_board[i][j] == 0) {
                printf("|    ");
            } else {
                printf("| %2d ", guessed_board[i][j]);
            }
        }
        printf("|\n");
    }
    printf("+----+----+----+----+----+\t+----+----+----+----+----+\n");
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    REQ_PACKET req;
    RES_PACKET res;

    int board[BOARD_SIZE][BOARD_SIZE];
    int guessed_board[BOARD_SIZE][BOARD_SIZE] = {0};
    int count = 0;

    fill_board(board);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_sock);
        exit(1);
    }

    if (listen(server_sock, 1) == -1) {
        perror("Listen failed");
        close(server_sock);
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    if ((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len)) == -1) {
        perror("Accept failed");
        close(server_sock);
        exit(1);
    }

    printf("---------------------------------\n");
    printf("           Start Game Server           \n");
    printf("---------------------------------\n");

    while (1) {
        recv(client_sock, &req, sizeof(req), 0);
        printf("[Server] Rx GAME_REQ(cmd: %d, num: %d)\n", req.cmd, req.num);
        
        if (req.cmd == GAME_REQ) {
            int found = 0;
            for (int i = 0; i < BOARD_SIZE; i++) {
                for (int j = 0; j < BOARD_SIZE; j++) {
                    if (board[i][j] == req.num && guessed_board[i][j] == 0) {
                        guessed_board[i][j] = req.num;
                        board[i][j] = 0;  
                        found = 1;
                        count++;
                    }
                }
            }
            res.cmd = GAME_RES;
            res.num = req.num;
            memcpy(res.game_board, guessed_board, sizeof(guessed_board));
            res.count = count;
            printf("[Server] Tx GAME_RES(cmd: %d, num: %d, count: %d)\n", res.cmd, res.num, res.count);
            send(client_sock, &res, sizeof(res), 0);
            display_board(board, guessed_board);

            sleep(1);

            if (count == BOARD_SIZE * BOARD_SIZE) {
                printf("No empty space.\n");
                res.cmd = GAME_END;
                send(client_sock, &res, sizeof(res), 0);
                printf("[Server] Tx GAME_END(cmd: %d)\n", res.cmd);
                req.cmd = GAME_END_ACK;
                printf("[Server] Tx GAME_END_ACK(cmd: %d)\n", req.cmd);
                break;
            }
        } else if (req.cmd == GAME_END_ACK) {
            printf("Exit Server Program\n");
            break;
        }
    }

    close(client_sock);
    close(server_sock);
    return 0;
}