// 2022427833 니스타
// Client-side code for connecting to the server, sending position requests,
// and receiving grid updates. This code interacts with the server to participate in the game.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define ROW 7   // Number of rows in the grid
#define COL 7   // Number of columns in the grid
#define PORT 9190 // Port number for server-client communication

typedef struct {
    int cmd; // Command type sent to the server
    int row; // Row selected by the client
    int col; // Column selected by the client
} REQ_PACKET;

typedef struct {
    int cmd; // Command type received from the server
    int board[ROW][COL]; // Current state of the grid received from the server
    int result; // Result of the client's last move (SUCCESS or FAIL)
} RES_PACKET;

int running = 1; // Flag to indicate whether the client is still running

// Function to receive messages from the server and process responses
void* recv_msg(void* arg) {
    int sock = *(int*)arg; // Socket file descriptor
    RES_PACKET res;

    while (running) {
        // Receive the response packet from the server
        if (recv(sock, &res, sizeof(res), 0) <= 0) {
            running = 0; // Stop if the connection is lost
            break;
        }

        // Display the updated grid state received from the server
        printf("+----------------------------------+\n");
        for (int i = 0; i < ROW; ++i) {
            printf("|");
            for (int j = 0; j < COL; ++j) {
                printf(" %2d |", res.board[i][j]);
            }
            printf("\n+----------------------------------+\n");
        }

        // Log the server's response
        printf("[Rx] cmd: %d, result: %d\n", res.cmd, res.result);

        // Check if the game has ended
        if (res.cmd == 3) { // Command for GAME_END
            printf("GAME_END. Game is over!\n");
            running = 0; // Stop the client
        }
    }
    return NULL;
}

// Function to send position requests to the server
void* send_msg(void* arg) {
    int sock = *(int*)arg; // Socket file descriptor
    REQ_PACKET req;

    srand(time(NULL)); // Seed the random number generator

    while (running) {
        req.cmd = 1; // Command for GAME_REQUEST
        req.row = rand() % ROW; // Randomly select a row
        req.col = rand() % COL; // Randomly select a column

        // Send the request packet to the server
        if (send(sock, &req, sizeof(req), 0) <= 0) {
            running = 0; // Stop if the connection is lost
            break;
        }

        // Log the request sent to the server
        printf("[Tx] cmd: %d, index(%d, %d)\n", req.cmd, req.row, req.col);

        sleep(1); // Pause for 1 second before the next move
    }
    return NULL;
}

// Main function to connect to the server and manage communication
int main(int argc, char* argv[]) {
    if (argc != 3) {
        // Display usage instructions if the required arguments are missing
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return -1;
    }

    int sock; // Socket file descriptor
    struct sockaddr_in server_addr; // Server address structure
    pthread_t recv_thread, send_thread; // Threads for sending and receiving messages

    // Create a socket for communication with the server
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed"); // Error message if socket creation fails
        return -1;
    }

    // Configure the server address
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(atoi(argv[2])); // Convert port to network byte order
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported"); // Error message if address is invalid
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed"); // Error message if connection fails
        return -1;
    }
    printf("Connected to server.\n");

    // Create threads for sending and receiving messages
    pthread_create(&recv_thread, NULL, recv_msg, &sock);
    pthread_create(&send_thread, NULL, send_msg, &sock);

    // Wait for both threads to finish
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);

    // Close the socket and terminate
    close(sock);
    return 0;
}
