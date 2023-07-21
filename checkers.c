#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 8
#define EMPTY_CELL 0
#define PLAYER1 1
#define PLAYER2 2

int board[BOARD_SIZE][BOARD_SIZE];

// initializing the board
void initializeBoard() {
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if ((row + col) % 2 == 0) {
                if (row < 3)
                    board[row][col] = PLAYER2; // player 1 pieces
                else if (row > BOARD_SIZE - 4)
                    board[row][col] = PLAYER1; // player 2 pieces
                else
                    board[row][col] = EMPTY_CELL; // empty
            } else {
                board[row][col] = EMPTY_CELL; // empty
            }
        }
    }
}

// printing the board
void printBoard() {
    for (int col = 0; col < BOARD_SIZE; ++col) {
        printf("----");
    }
    
    printf("\n");

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (board[row][col] == 1) {
                printf("| X ");
            } else if (board[row][col] == 2) {
                printf("| O ");
            } else {
                printf("|   ");
            }
        }
        printf("|\n");
        for (int col = 0; col < BOARD_SIZE; ++col) {
            printf("----");
        }
        printf("\n");
    }
}

int main() {
    initializeBoard();
    printBoard();

    return 0;
}
