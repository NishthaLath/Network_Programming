#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define ROW 7
#define COL 7
#define PORT 9190
#define MAX_CLIENTS 3

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

int board[ROW][COL];
pthread_mutex_t board_mutex;

void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);
    REQ_PACKET req;
    RES_PACKET res;
    
    while (1) {
        // Receive client request
        if (recv(client_sock, &req, sizeof(req), 0) <= 0) break;

        pthread_mutex_lock(&board_mutex);
        
        // Check if position is already occupied
        if (board[req.row][req.col] != 0) {
            res.cmd = 2;
            res.result = 0; // FAIL
        } else {
            // Mark position and send SUCCESS
            board[req.row][req.col] = client_sock;
            res.cmd = 2;
            res.result = 1; // SUCCESS
        }
        memcpy(res.board, board, sizeof(board));
        pthread_mutex_unlock(&board_mutex);
        
        // Send response to client
        send(client_sock, &res, sizeof(res), 0);
    }
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock, client_sock, addr_size;
    struct sockaddr_in server_addr, client_addr;
    pthread_t threads[MAX_CLIENTS];

    // Initialize board
    memset(board, 0, sizeof(board));
    pthread_mutex_init(&board_mutex, NULL);

    // Set up server socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, MAX_CLIENTS);

    printf("Server started on port %d\n", PORT);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, (socklen_t*)&addr_size);
        printf("Client connected: %d\n", client_sock);
        
        int* new_sock = malloc(sizeof(int));
        *new_sock = client_sock;
        pthread_create(&threads[i], NULL, handle_client, new_sock);
    }

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&board_mutex);
    close(server_sock);
    return 0;
}
