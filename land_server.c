// 2022427833 니스타
// Server-side code for managing a game where clients select positions on a shared grid.
// The server calculates and visualizes contiguous regions for each client.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define ROW 7           // Number of rows in the grid
#define COL 7           // Number of columns in the grid
#define PORT 9190       // Port number for server-client communication
#define MAX_CLIENTS 3   // Maximum number of clients that can connect simultaneously

typedef struct {
    int cmd; // Command type sent by the client
    int row; // Row selected by the client
    int col; // Column selected by the client
} REQ_PACKET;

typedef struct {
    int cmd;           // Command type sent by the server
    int board[ROW][COL]; // Current state of the grid
    int result;        // Result of the client's move (SUCCESS or FAIL)
} RES_PACKET;

int board[ROW][COL];           // Shared grid between clients
pthread_mutex_t board_mutex;   // Mutex for synchronizing access to the grid
int occupied_cells = 0;        // Counter to track the number of occupied cells

// Function to display the current state of the game board
void print_board() {
    printf("+----------------------------------+\n");
    for (int i = 0; i < ROW; ++i) {
        printf("|");
        for (int j = 0; j < COL; ++j) {
            printf(" %2d |", board[i][j]);
        }
        printf("\n+----------------------------------+\n");
    }
    printf("Occupied: %d\n", occupied_cells);
}

// Check if a cell has at least one valid horizontal/vertical neighbor
int Valid_Space(int x, int y, int client_id) {
    int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; // Horizontal and vertical directions

    for (int d = 0; d < 4; ++d) {
        int nx = x + directions[d][0];
        int ny = y + directions[d][1];

        if (nx >= 0 && nx < ROW && ny >= 0 && ny < COL && board[nx][ny] == client_id) {
            return 1; // Valid neighbor found
        }
    }
    return 0; // No valid neighbors
}

// Calculate and visualize contiguous regions for a client
int Continuous_Space(int client_id, int output_board[ROW][COL]) {
    int visited[ROW][COL] = {0}; // Keep track of visited cells
    int total_size = 0;          // Total size of the contiguous region

    for (int i = 0; i < ROW; ++i) {
        for (int j = 0; j < COL; ++j) {
            if (board[i][j] == client_id && !visited[i][j]) {
                if (Valid_Space(i, j, client_id)) {
                    visited[i][j] = 1;          // Mark as visited
                    output_board[i][j] = client_id; // Mark in output board
                    total_size++;
                }
            }
        }
    }
    return total_size;
}

// Display all contiguous regions for each client
void calculate_regions() {
    for (int id = 4; id <= 6; ++id) {
        printf("[Client %d] Continuous Space\n", id);

        int output_board[ROW][COL] = {0};
        int total_size = Continuous_Space(id, output_board);

        printf("+----------------------------------+\n");
        for (int i = 0; i < ROW; ++i) {
            printf("|");
            for (int j = 0; j < COL; ++j) {
                if (output_board[i][j] == id) {
                    printf(" %2d |", id); // Show the client ID
                } else {
                    printf("    |"); // Empty space
                }
            }
            printf("\n+----------------------------------+\n");
        }
        printf("Space size: %d\n\n", total_size);
    }
}

// Handle a client's requests
void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);
    REQ_PACKET req;
    RES_PACKET res;

    while (1) {
        if (recv(client_sock, &req, sizeof(req), 0) <= 0) break;

        pthread_mutex_lock(&board_mutex);
        if (board[req.row][req.col] != 0) {
            res.cmd = 2; // Command for FAILURE
            res.result = 0;
        } else {
            board[req.row][req.col] = client_sock; // Mark the cell with the client ID
            res.cmd = 2; // Command for SUCCESS
            res.result = 1;
            occupied_cells++;
        }
        memcpy(res.board, board, sizeof(board));
        pthread_mutex_unlock(&board_mutex);

        send(client_sock, &res, sizeof(res), 0);
        printf("[Tx] cmd: %d, client_id: %d, result: %d\n", res.cmd, client_sock, res.result);
        print_board();

        if (occupied_cells == ROW * COL) {
            res.cmd = 3; // Command for GAME_END
            printf("[Tx] cmd: %d, client_id: %d, result: %d\n", res.cmd, client_sock, res.result);
            printf("Game board is full. Game is over.\n");
            send(client_sock, &res, sizeof(res), 0);
            break;
        }
    }
    close(client_sock);
    return NULL;
}

// Main server function
int main() {
    int server_sock, client_sock, addr_size;
    struct sockaddr_in server_addr, client_addr;
    pthread_t threads[MAX_CLIENTS];

    memset(board, 0, sizeof(board));
    pthread_mutex_init(&board_mutex, NULL);

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

    calculate_regions();
    pthread_mutex_destroy(&board_mutex);
    close(server_sock);
    return 0;
}
