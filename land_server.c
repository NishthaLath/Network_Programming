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
int occupied_cells = 0;

// Function to display the game board
void print_board() {
    printf("+-------------------------------+\n");
    for (int i = 0; i < ROW; ++i) {
        printf("|");
        for (int j = 0; j < COL; ++j) {
            printf(" %2d |", board[i][j]);
        }
        printf("\n+-------------------------------+\n");
    }
    printf("Occupied: %d\n", occupied_cells);
}

// Helper function for flood fill
void flood_fill(int x, int y, int client_id, int visited[ROW][COL], int output_board[ROW][COL], int *region_size) {
    if (x < 0 || x >= ROW || y < 0 || y >= COL || visited[x][y] || board[x][y] != client_id) {
        return;
    }

    visited[x][y] = 1;
    output_board[x][y] = client_id;
    (*region_size)++;

    // Explore neighbors (horizontally and vertically)
    flood_fill(x + 1, y, client_id, visited, output_board, region_size);
    flood_fill(x - 1, y, client_id, visited, output_board, region_size);
    flood_fill(x, y + 1, client_id, visited, output_board, region_size);
    flood_fill(x, y - 1, client_id, visited, output_board, region_size);
}

int Continuous_space(int client_id, int output_board[ROW][COL]) {
    int visited[ROW][COL] = {0}; // To mark cells already part of a region
    int total_size = 0;

    for (int i = 0; i < ROW; ++i) {
        for (int j = 0; j < COL; ++j) {
            // Check if the cell belongs to the current client ID and hasn't been visited
            if (board[i][j] == client_id && !visited[i][j]) {
                int region_size = 0;
                flood_fill(i, j, client_id, visited, output_board, &region_size);
                total_size += region_size;
            }
        }
    }

    return total_size;
}

// Calculate and display contiguous regions visually
void calculate_regions() {
    for (int id = 4; id <= 6; ++id) {
        printf("[Client %d] Continuous Space\n", id);

        int output_board[ROW][COL] = {0};
        int total_size = Continuous_space(id, output_board);

        // Display the grid visually for the client
        printf("+-------------------------------+\n");
        for (int i = 0; i < ROW; ++i) {
            printf("|");
            for (int j = 0; j < COL; ++j) {
                if (output_board[i][j] == id) {
                    printf(" %2d |", id); // Show the client ID in the contiguous region
                } else {
                    printf("    |"); // Empty space elsewhere
                }
            }
            printf("\n+-------------------------------+\n");
        }
        printf("Space size: %d\n\n", total_size);
    }
}

void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);
    REQ_PACKET req;
    RES_PACKET res;

    while (1) {
        if (recv(client_sock, &req, sizeof(req), 0) <= 0) break;

        pthread_mutex_lock(&board_mutex);
        if (board[req.row][req.col] != 0) {
            res.cmd = 2;
            res.result = 0;
        } else {
            board[req.row][req.col] = client_sock;
            res.cmd = 2;
            res.result = 1;
            occupied_cells++;
        }
        memcpy(res.board, board, sizeof(board));
        pthread_mutex_unlock(&board_mutex);

        send(client_sock, &res, sizeof(res), 0);
        print_board();

        if (occupied_cells == ROW * COL) {
            res.cmd = 3; // GAME_END
            send(client_sock, &res, sizeof(res), 0);
            break;
        }
    }
    close(client_sock);
    return NULL;
}

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