#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 8
#define EMPTY_CELL 0
#define PLAYER1 1
#define PLAYER2 2

void initializeBoard();
void printBoard();
int isNotWithinBounds(int toRow, int toCol);
int isValidMove(int turn, int fromRow, int fromCol, int toRow, int toCol);
void makeMove(int turn, int fromRow, int fromCol, int toRow, int toCol);
int isGameOver();
int getPlayerMove(int turn, int* fromRow, int* fromCol, int* toRow, int* toCol);

int board[BOARD_SIZE][BOARD_SIZE];
int turn = PLAYER1;

int main() {
	initializeBoard();
	printBoard();

	int fromRow, fromCol, toRow, toCol;

	// main game loop
	while (!isGameOver()) {
		if (getPlayerMove(turn, &fromRow, &fromCol, &toRow, &toCol)) {
			makeMove(turn, fromRow, fromCol, toRow, toCol);
			printBoard();
			turn = (turn == PLAYER1) ? PLAYER2 : PLAYER1;
		} else {
			printf("Invalid move. Try again.\n");
		}
	}

	// if game is over, check the winner
	if (isGameOver()) {
		int player1Pieces = 0, player2Pieces = 0;
		for (int row = 0; row < BOARD_SIZE; ++row) {
			for (int col = 0; col < BOARD_SIZE; ++col) {
				if (board[row][col] == PLAYER1 || board[row][col] == PLAYER1 + 2)
					player1Pieces++;
				else if (board[row][col] == PLAYER2 || board[row][col] == PLAYER2 + 2)
					player2Pieces++;
			}
		}

		if (player1Pieces > player2Pieces)
			printf("Player 1 wins!\n");
		else if (player2Pieces > player1Pieces)
			printf("Player 2 wins!\n");
		else
			printf("It's a draw!\n");
	}

	return 0;
}

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
	printf("    ");
	for (int col = 0; col < BOARD_SIZE; ++col) {
		printf("  %d ", col);
	}

	printf("\n    ");

	for (int col = 0; col < BOARD_SIZE; ++col) {
		printf("----");
	}
    
	printf("\n");

	for (int row = 0; row < BOARD_SIZE; ++row) {
		printf("  %d ", row);

		for (int col = 0; col < BOARD_SIZE; ++col) {
			// player 1 piece
			if (board[row][col] == PLAYER1) {
				printf("| x ");
			// player 2 piece
			} else if (board[row][col] == PLAYER2) {
				printf("| o ");
			// player 1 king
			} else if (board[row][col] == PLAYER1 + 2) {
				printf("| X ");
			// player 2 king
			} else if (board[row][col] == PLAYER2 + 2) {
				printf("| O ");
			// empty cell
			} else {
				printf("|   ");
			}
		}
    
		printf("|\n    ");

		for (int col = 0; col < BOARD_SIZE; ++col) {
			printf("----");
		}
			
		printf("\n");
	}
}

// check if not within the bounds of the board
int isNotWithinBounds(int toRow, int toCol) {
	return toRow < 0 || toRow >= BOARD_SIZE || toCol < 0 || toCol >= BOARD_SIZE;
}

// check if a move is valid
int isValidMove(int player, int fromRow, int fromCol, int toRow, int toCol) {
	// check if the destination is within the bounds of the board
	if (isNotWithinBounds(toRow, toCol)) return 0;

	// check if the destination is empty
	if (board[toRow][toCol] != EMPTY_CELL) return 0;

	// check if the piece is moving diagonally
	//if (fromRow == toRow || fromCol == toCol) return 0;

	// check if the piece is moving diagonally
	//int rowDiff = abs(toRow - fromRow);
	//int colDiff = abs(toCol - fromCol);
	//if (rowDiff != colDiff) return 0;

	// check if the piece is moving diagonally
	int rowDiff = abs(toRow - fromRow);
	int colDiff = abs(toCol - fromCol);
	if (rowDiff == 0 || colDiff == 0 || rowDiff != colDiff) return 0;

	// check if the piece is moving in the correct direction
	int direction = (player == PLAYER1) ? -1 : 1;
	int rowMove = toRow - fromRow;
	if (rowMove * direction < 0) return 0;

	// check the distance of the move to see if is regular to capture move
	int distance = abs(toCol - fromCol) > abs(toRow - fromRow) ? abs(toCol - fromCol) : abs(toRow - fromRow);
	if (distance != 1 && distance != 2) return 0;

	// if it is a capture move, check if the opponent piece is in the middle
	if (distance == 2) {
		int midRow = (fromRow + toRow) / 2;
		int midCol = (fromCol + toCol) / 2;

		if (board[midRow][midCol] != (player % 2) + 1) return 0;
	}

	return 1; // move is valid
}

// function to update the board after a valid move
void makeMove(int turn, int fromRow, int fromCol, int toRow, int toCol) {
	// move the piece to the destination cell
	board[toRow][toCol] = board[fromRow][fromCol];
	board[fromRow][fromCol] = EMPTY_CELL;

	// check if the piece should be promoted to a king
	if ((turn == PLAYER1 && toRow == 0) ||
		(turn == PLAYER2 && toRow == BOARD_SIZE - 1)) {
		board[toRow][toCol] += 2; // promote the piece to a king
	}

	// maybe change this ???????????????????
	// if it is a capture move, remove the captured piece
	if (abs(toRow - fromRow) == 2) {
		int midRow = (fromRow + toRow) / 2;
		int midCol = (fromCol + toCol) / 2;
		board[midRow][midCol] = EMPTY_CELL;
	}
}

// check if the game has ended
int isGameOver() {
	// check if a player has lost all their pieces
	int player1Pieces = 0, player2Pieces = 0;
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if (board[row][col] == PLAYER1 || board[row][col] == PLAYER1 + 2)
				player1Pieces++;
			else if (board[row][col] == PLAYER2 || board[row][col] == PLAYER2 + 2)
				player2Pieces++;
			}
	}

	if (player1Pieces == 0 || player2Pieces == 0)
		return 1;

	// check for a stalemate (no more possible moves)
	int currentPlayerPieces = (player1Pieces > player2Pieces) ? PLAYER1 : PLAYER2;
	int stalemate = 1;
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if (board[row][col] == currentPlayerPieces) {
				// Check if the current player has any valid moves
				if ((row > 0 && col > 0 && isValidMove(currentPlayerPieces, row, col, row - 1, col - 1)) ||
					(row > 0 && col < BOARD_SIZE - 1 && isValidMove(currentPlayerPieces, row, col, row - 1, col + 1)) ||
					(row < BOARD_SIZE - 1 && col > 0 && isValidMove(currentPlayerPieces, row, col, row + 1, col - 1)) ||
					(row < BOARD_SIZE - 1 && col < BOARD_SIZE - 1 && isValidMove(currentPlayerPieces, row, col, row + 1, col + 1))
				) {
					stalemate = 0;
					break;
				}
			}
		}
			
		if (!stalemate)
			break;
	}

	if (stalemate)
		return 1;

	// maybe add more conditions ????????

	return 0; // game is not over
}

// function to prompt the player for their move and validate the input
int getPlayerMove(int turn, int* fromRow, int* fromCol, int* toRow, int* toCol) {
	if (turn == PLAYER1) {
		printf("Player1(X) turn:\n");
	} else {
		printf("Player2(O) turn:\n");
	}
	printf("Enter your move (fromRow fromCol toRow toCol): ");
	fflush(stdout);

	if (scanf("%d %d %d %d", fromRow, fromCol, toRow, toCol) != 4) {
		while (getchar() != '\n');
		return 0; // invalid input
	}

	// validate the input positions
	if ((*fromRow < 0 || *fromRow >= BOARD_SIZE) || (*fromCol < 0 || *fromCol >= BOARD_SIZE) ||
		(*toRow < 0 || *toRow >= BOARD_SIZE) || (*toCol < 0 || *toCol >= BOARD_SIZE)) {
		return 0; // invalid input positions
	}

	// validate the move
	if (!isValidMove(turn, *fromRow, *fromCol, *toRow, *toCol)) {
		return 0; // invalid move
	}

	return 1; // valid input
}