// 2022427833 - Lath Nishtha
// tic-tac-toe server program using UDP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

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
int available_space(GAMEBOARD *gboard);
void server_make_move(GAMEBOARD *gboard, int *row, int *col); 

int main(int argc, char *argv[]) {
    int serv_sock;
    socklen_t clnt_adr_sz;
    struct sockaddr_in serv_adr, clnt_adr;
    GAMEBOARD gboard = {0}; // Initialize the game board with zeros
    int row, col;  // Variables to store the server's move

    // Check if the port number is provided
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    
    // Create a UDP socket
    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");
    
    // Initialize server address structure
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));
    
    // Bind the socket to the server address
    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    printf("Tic-Tac-Toe Server\n");
    
    while (1) {
        clnt_adr_sz = sizeof(clnt_adr);
        // Receive the game board from the client
        int str_len = recvfrom(serv_sock, &gboard, sizeof(gboard), 0, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        if (str_len == -1)
            error_handling("recvfrom() error");

        printf("Client's move:\n");
        draw_board(&gboard);

        // Check if there is available space on the board
        if (!available_space(&gboard)) {
            printf("No available space. Game over.\n");
            printf("Tic Tac Toe Server Close\n");
            break;
        }

        // Server makes a move and we get the row and column
        server_make_move(&gboard, &row, &col);
        
        // Display the server's move
        printf("Server chose: [%d, %d]\n", row, col);
        draw_board(&gboard);

        // Send the updated game board back to the client
        sendto(serv_sock, &gboard, sizeof(gboard), 0, (struct sockaddr*)&clnt_adr, clnt_adr_sz);

        // Check again if there is available space on the board
        if (!available_space(&gboard)) {
            printf("No available space. Game over.\n");
            printf("Tic Tac Toe Server Close\n");
            break;
        }
    }

    close(serv_sock);
    return 0;
}

// Function for the server to make a move
void server_make_move(GAMEBOARD *gboard, int *row, int *col) {
    srand(time(NULL));  // Initialize random seed
    do {
        *row = rand() % BOARD_SIZE;
        *col = rand() % BOARD_SIZE;
    } while (gboard->board[*row][*col] != INIT_VALUE);  // Find an empty spot
    gboard->board[*row][*col] = S_VALUE;  // Server places 'O'
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