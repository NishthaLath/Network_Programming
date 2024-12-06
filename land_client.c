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

// Function to receive messages from the server
void* recv_msg(void* arg) {
    int sock = *(int*)arg;
    RES_PACKET res;
    
    while (running) {
        // Receive server response
        if (recv(sock, &res, sizeof(res), 0) <= 0) {
            running = 0;
            break;
        }

        // Print the board state
        printf("+----------------------------------+\n");
        for (int i = 0; i < ROW; ++i) {
            printf("|");
            for (int j = 0; j < COL; ++j) {
                printf(" %2d |", res.board[i][j]);
            }
            printf("\n+----------------------------------+\n");
        }
        
        // Check for GAME_END signal from server
        if (res.cmd == 3) {
            printf("GAME_END. Game is over!\n");
            running = 0;
        }
    }
    return NULL;
}

// Function to send messages to the server
void* send_msg(void* arg) {
    int sock = *(int*)arg;
    REQ_PACKET req;
    
    srand(time(NULL)); // Initialize random number generator
    
    while (running) {
        req.cmd = 1; // GAME_REQUEST
        req.row = rand() % ROW;
        req.col = rand() % COL;

        // Send the randomly selected position to the server
        if (send(sock, &req, sizeof(req), 0) <= 0) {
            running = 0;
            break;
        }
        sleep(1); // Pause for 1 second before the next move
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
    
    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }
    
    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        return -1;
    }
    printf("Connected to server.\n");

    // Create threads for sending and receiving messages
    pthread_create(&recv_thread, NULL, recv_msg, &sock);
    pthread_create(&send_thread, NULL, send_msg, &sock);
    
    // Wait for threads to complete
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);
    
    close(sock);
    return 0;
}
