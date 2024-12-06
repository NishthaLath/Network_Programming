#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define ROW 7
#define COL 7
#define PORT 9190

typedef struct {
    int cmd; // Command value
    int row; // Random row selected by the client
    int col; // Random column selected by the client
} REQ_PACKET;

typedef struct {
    int cmd; // Command value
    int board[ROW][COL]; // 7x7 grid representing the board state
    int result; // Result value (FAIL or SUCCESS)
} RES_PACKET;

int running = 1;

void* recv_msg(void* arg) {
    int sock = *(int*)arg;
    RES_PACKET res;
    
    while (running) {
        // Receive server response
        if (recv(sock, &res, sizeof(res), 0) <= 0) break;

        // Print board state and result
        printf("+-------------------------------+\n");
        for (int i = 0; i < ROW; ++i) {
            printf("|");
            for (int j = 0; j < COL; ++j) {
                printf(" %2d |", res.board[i][j]);
            }
            printf("\n+-------------------------------+\n");
        }
        if (res.cmd == 3) { // GAME_END
            printf("GAME_END. Game is over!\n");
            running = 0;
        }
    }
    return NULL;
}

void* send_msg(void* arg) {
    int sock = *(int*)arg;
    REQ_PACKET req;
    
    srand(time(NULL));
    while (running) {
        req.cmd = 1; // GAME_REQUEST
        req.row = rand() % ROW;
        req.col = rand() % COL;
        
        // Send random position to server
        send(sock, &req, sizeof(req), 0);
        sleep(1);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return -1;
    }
    
    int sock;
    struct sockaddr_in server_addr;
    pthread_t recv_thread, send_thread;
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    
    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed.\n");
        return -1;
    }
    printf("Connected to server.\n");

    // Start threads for sending and receiving messages
    pthread_create(&recv_thread, NULL, recv_msg, &sock);
    pthread_create(&send_thread, NULL, send_msg, &sock);
    
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);
    
    close(sock);
    return 0;
}
