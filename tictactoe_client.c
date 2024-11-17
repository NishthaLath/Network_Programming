// 2022427833 - Lath Nishtha
// tic-tac-toe server program using UDP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#define BUF_SIZE 30
#define BOARD_SIZE 3
#define INIT_VALUE 0    // Empty space is represented by 0
#define S_VALUE 1       // Server's move is represented by 1
#define C_VALUE 2       // Client's move is represented by 2

typedef struct {
    int board[BOARD_SIZE][BOARD_SIZE];
} GAMEBOARD;

void error_handling(char *message);
void draw_board(GAMEBOARD *gboard);
int get_valid_input(int *row, int *col);
int available_space(GAMEBOARD *gboard);

int main(int argc, char *argv[]) {
    int sock;
    socklen_t adr_sz;
    struct sockaddr_in serv_adr, from_adr;
    GAMEBOARD gboard = {0};  // Initialize the game board with zeros
    char message[BUF_SIZE];  // Buffer to receive messages

    // Check if the IP address and port number are provided
    if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // Create a UDP socket
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    // Initialize server address structure
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    // Start the game
    printf("Tic-Tac-Toe Client\n");

    while (1) {
        // Display the current game board
        draw_board(&gboard);

        int row, col;
        // Get valid input from the user
        if (get_valid_input(&row, &col) == -1) {
            printf("Invalid input. Try again.\n");
            continue;
        }

        // Check if the input is valid in terms of board position
        if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE || gboard.board[row][col] != INIT_VALUE) {
            printf("Wrong Index. Input again\n");
            continue;
        }

        // Update board with client's move
        gboard.board[row][col] = C_VALUE;  // Client makes its move ('X')
        
        // Check if the board is full after the client's move
        if (!available_space(&gboard)) {
            draw_board(&gboard);
            printf("No available space. Exit client.\n");
            printf("Tic Tac Toe Client Close\n");
            sendto(sock, &gboard, sizeof(gboard), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
            break;  // Exit the loop, no need to contact the server as it's the final move
        }

        // Send updated board to the server
        sendto(sock, &gboard, sizeof(gboard), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));

        // Receive the server's move
        adr_sz = sizeof(from_adr);
        int str_len = recvfrom(sock, &gboard, sizeof(gboard), 0, (struct sockaddr*)&from_adr, &adr_sz);
        if (str_len == -1)
            error_handling("recvfrom() error");

        // Display the updated game board after the server's move
        draw_board(&gboard);
        printf("Server's move:\n");

        // Check if the board is full after the server's move
        if (!available_space(&gboard)) {
            printf("No available space. Exit client.\n");
            printf("Tic Tac Toe Client Close\n");
            break;  // Exit the loop
        }
    }

    close(sock);
    return 0;
}

// Function to get valid input from the user
int get_valid_input(int *row, int *col) {
    char input[100];

    // Read the entire line of input
    printf("Enter your move (row and column): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return -1;  // If there was an input error
    }

    // Parse input using sscanf
    if (sscanf(input, "%d %d", row, col) != 2) {
        // If sscanf doesn't successfully read two integers, return error
        return -1;
    }

    return 0;  // Valid input
}

// Function to check if there is available space on the board
int available_space(GAMEBOARD *gboard) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gboard->board[i][j] == INIT_VALUE)
                return 1;  // There is space
        }
    }
    return 0;  // No space available
}

// Function to draw the game board
void draw_board(GAMEBOARD *gboard) {
    char value;
    printf("     0     1     2  \n");  // Column labels
    printf("   +-----+-----+-----+\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf(" %d ", i);  // Row label
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gboard->board[i][j] == INIT_VALUE)
                value = ' ';  // Empty space
            else if (gboard->board[i][j] == S_VALUE)
                value = 'O';  // Server's move
            else if (gboard->board[i][j] == C_VALUE)
                value = 'X';  // Client's move
            
            printf("|  %c  ", value);  // Print cell with padding
        }
        printf("|\n");  // End of row
        printf("   +-----+-----+-----+\n");  // Divider between rows
    }
}

// Function to handle errors
void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}