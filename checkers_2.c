#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>

#define BOARD_SIZE 8
#define EMPTY_CELL 0
#define PLAYER1 1
#define PLAYER2 2

void initializeBoard(int board[BOARD_SIZE][BOARD_SIZE]);
void printBoard(int board[BOARD_SIZE][BOARD_SIZE]);
void copyBoard(int src[BOARD_SIZE][BOARD_SIZE], int dest[BOARD_SIZE][BOARD_SIZE]);
int isNotWithinBounds(int toRow, int toCol);
int isValidMove(int board[BOARD_SIZE][BOARD_SIZE], int turn, int fromRow, int fromCol, int toRow, int toCol);
void makeMove(int board[BOARD_SIZE][BOARD_SIZE], int turn, int fromRow, int fromCol, int toRow, int toCol);
int isGameOver(int board[BOARD_SIZE][BOARD_SIZE]);
int getPlayerMove(int board[BOARD_SIZE][BOARD_SIZE], int turn, int* fromRow, int* fromCol, int* toRow, int* toCol);
void getPossibleMoves(int board[BOARD_SIZE][BOARD_SIZE], int turn, int possibleMoves[100][4], int* numMoves);
int hasValidMoves(int board[BOARD_SIZE][BOARD_SIZE], int turn);
int evaluatePosition(int board[BOARD_SIZE][BOARD_SIZE]);
int minimax(int board[BOARD_SIZE][BOARD_SIZE], int maxDepth, int depth, int turn, int alpha, int beta);
int getBestMoveForOpponent(int board[BOARD_SIZE][BOARD_SIZE], int turn, int maxDepth, int* fromRow, int* fromCol, int* toRow, int* toCol, int* score, int rank, int numProcesses);

int main(int argc, char** argv) {
	int board[BOARD_SIZE][BOARD_SIZE];
	int turn = PLAYER1;
	int maxDepth;
	int fromRow, fromCol, toRow, toCol;

	initializeBoard(board);

	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	int numProcesses;
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

	if (rank == 0) {
		printf("Enter the max depth to be searched: ");
		fflush(stdout);
		scanf("%d", &maxDepth);
		getchar();

		printBoard(board);
	}

	MPI_Bcast(&maxDepth, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// main game loop
	while (!isGameOver(board)) {
		// my turn
		if (turn == PLAYER1) {
			if (rank == 0) {
				if (getPlayerMove(board, turn, &fromRow, &fromCol, &toRow, &toCol)) {
					makeMove(board, turn, fromRow, fromCol, toRow, toCol);
					printBoard(board);

					turn = (turn == PLAYER1) ? PLAYER2 : PLAYER1;
				} else {
					printf("Invalid move. Try again.\n");
				}
			}

			MPI_Bcast(&board, 64, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&turn, 1, MPI_INT, 0, MPI_COMM_WORLD);

			// AI turn
		}	else {
			int score;
			int scores[numProcesses];
			int play = 0;

			getBestMoveForOpponent(board, turn, maxDepth, &fromRow, &fromCol, &toRow, &toCol, &score, rank, numProcesses);
			
			MPI_Gather(&score, 1, MPI_INT, scores, 1, MPI_INT, 0, MPI_COMM_WORLD);

			if (rank == 0) {
				int bestScore = -9999;

				for (int i = 0; i < numProcesses; ++i) {
					if (bestScore < scores[i]) {
						bestScore = scores[i];
						play = i;
					}
				}
			} 
				
			MPI_Bcast(&play, 1, MPI_INT, 0, MPI_COMM_WORLD);

			if (rank == play) {
				makeMove(board, turn, fromRow, fromCol, toRow, toCol);
				printf("Player 2(O) move: %d %d %d %d\n", fromRow, fromCol, toRow, toCol);
				printBoard(board);
				turn = (turn == PLAYER1) ? PLAYER2 : PLAYER1;
			}

			MPI_Bcast(&board, 64, MPI_INT, play, MPI_COMM_WORLD);
			MPI_Bcast(&turn, 1, MPI_INT, play, MPI_COMM_WORLD);
		}
	}

	if (rank == 0) {
		// if game is over, check the winner
		if (isGameOver(board)) {
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
				printf("Player 1(X) wins!\n");
			else if (player2Pieces > player1Pieces)
				printf("Player 2(O) wins!\n");
			else
				printf("Draw!\n");
		}
	}

	MPI_Finalize();

	return 0;
}

// initializing the board
void initializeBoard(int board[BOARD_SIZE][BOARD_SIZE]) {
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if ((row + col) % 2 == 1) {
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
void printBoard(int board[BOARD_SIZE][BOARD_SIZE]) {
	printf("\n");
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

	printf("\n");
}

// create a copy of the board
void copyBoard(int src[BOARD_SIZE][BOARD_SIZE], int dest[BOARD_SIZE][BOARD_SIZE]) {
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			dest[row][col] = src[row][col];
		}
	}
}

// check if not within the bounds of the board
int isNotWithinBounds(int toRow, int toCol) {
	return toRow < 0 || toRow >= BOARD_SIZE || toCol < 0 || toCol >= BOARD_SIZE;
}

// check if a move is valid
int isValidMove(int board[BOARD_SIZE][BOARD_SIZE], int turn, int fromRow, int fromCol, int toRow, int toCol) {
	// check if the destination is within the bounds of the board
	if (isNotWithinBounds(toRow, toCol)) return 0;

	// check if the cell selected not is empty
	if (board[fromRow][fromCol] == EMPTY_CELL) return 0;

	// check if the destination is empty
	if (board[toRow][toCol] != EMPTY_CELL) return 0;

	// check if the piece is moving diagonally
	int rowDiff = abs(toRow - fromRow);
	int colDiff = abs(toCol - fromCol);
	if (rowDiff == 0 || colDiff == 0 || rowDiff != colDiff) return 0;

	// check if is a king move
	int isKing = (board[fromRow][fromCol] == turn + 2) ? 1 : 0;

	// check if the piece is moving in the correct direction
	int direction = (turn == PLAYER1) ? -1 : 1;
	int rowMove = toRow - fromRow;
	if (rowMove * direction < 0 && !isKing) return 0;

	// check the distance of the move to see if is regular to capture move
	int distance = abs(toCol - fromCol) > abs(toRow - fromRow) ? abs(toCol - fromCol) : abs(toRow - fromRow);
	if (distance != 1 && distance != 2 && !isKing) return 0;

	// if it is a capture move, check if the opponent piece is in the middle
	if (distance == 2 && !isKing) {
		int midRow = (fromRow + toRow) / 2;
		int midCol = (fromCol + toCol) / 2;

		// check if the middle cell contains an opponent's piece or opponent king
		if (board[midRow][midCol] == turn || board[midRow][midCol] == turn + 2 || board[midRow][midCol] == EMPTY_CELL)
			return 0;
	}

	// check for conditions if it is a king piece
	if (isKing) {
		int rowDiff = abs(toRow - fromRow);
		int colDiff = abs(toCol - fromCol);
		if (rowDiff != colDiff) return 0;

		// check if the path is clear for a king move
		int i, row, col;
		int rowDir = (toRow - fromRow) / rowDiff; // 1 or -1
		int colDir = (toCol - fromCol) / colDiff; // 1 or -1
		for (i = 1; i < rowDiff - 1; i++) {
			row = fromRow + i * rowDir;
			col = fromCol + i * colDir;
			if (board[row][col] != EMPTY_CELL) return 0;
		}
	}

	return 1; // move is valid
}
 
// function to update the board after a valid move
void makeMove(int board[BOARD_SIZE][BOARD_SIZE], int turn, int fromRow, int fromCol, int toRow, int toCol) {
	int isKing = (board[fromRow][fromCol] == turn + 2) ? 1 : 0;

	// move the piece to the destination cell
	board[toRow][toCol] = board[fromRow][fromCol];
	board[fromRow][fromCol] = EMPTY_CELL;

	// check if the piece should be promoted to a king
	if ((turn == PLAYER1 && toRow == 0) || (turn == PLAYER2 && toRow == BOARD_SIZE - 1)) {
		// check if the piece is not already a king before promoting it

		if (board[toRow][toCol] == PLAYER1 || board[toRow][toCol] == PLAYER2) {
			board[toRow][toCol] += 2; // promote the piece to a king
		}
	}

	// if it is a capture move, remove the captured piece
	if (abs(toRow - fromRow) == 2 && !isKing) {
		int midRow = (fromRow + toRow) / 2;
		int midCol = (fromCol + toCol) / 2;
		board[midRow][midCol] = EMPTY_CELL;
	}

	// capture move for a king piece
	if (isKing) {
		int rowDiff = abs(toRow - fromRow);
		int colDiff = abs(toCol - fromCol);

		// check if the path is clear for a king move
		int rowDir = (toRow - fromRow) / rowDiff; // 1 or -1
		int colDir = (toCol - fromCol) / colDiff; // 1 or -1
		board[toRow-rowDir][toCol-colDir] = EMPTY_CELL;
	}
}

// check if the game has ended
int isGameOver(int board[BOARD_SIZE][BOARD_SIZE]) {
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

	// if a player has no pieces the game is over
	if (player1Pieces == 0 || player2Pieces == 0) return 1;

	// check for stalemate, if so it is a draw
	if (!hasValidMoves(board, PLAYER1) || !hasValidMoves(board, PLAYER2)) {
		return 1;
	}

	return 0; // game is not over
}

// function to prompt the player for their move and validate the input
int getPlayerMove(int board[BOARD_SIZE][BOARD_SIZE], int turn, int* fromRow, int* fromCol, int* toRow, int* toCol) {
	if (turn == PLAYER1) {
		printf("Player 1(X) turn:\n");
	} else {
		printf("Player 2(O) turn:\n");
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
	if (!isValidMove(board, turn, *fromRow, *fromCol, *toRow, *toCol)) {
		return 0; // invalid move
	}

	return 1; // valid input
}

// check if a player has any valid moves left
int hasValidMoves(int board[BOARD_SIZE][BOARD_SIZE], int turn) {
	// loop through the board and find all possible moves for the current player
	for (int fromRow = 0; fromRow < BOARD_SIZE; ++fromRow) {
		for (int fromCol = 0; fromCol < BOARD_SIZE; ++fromCol) {
			if (board[fromRow][fromCol] == turn || board[fromRow][fromCol] == turn + 2) {
				for (int toRow = 0; toRow < BOARD_SIZE; ++toRow) {
					for (int toCol = 0; toCol < BOARD_SIZE; ++toCol) {
						if (isValidMove(board, turn, fromRow, fromCol, toRow, toCol)) {
							return 1; // found a valid move
						}
					}
				}
			}
		}
	}

	return 0; // no valid moves
}

// generate all possible moves for a player
void getPossibleMoves(int board[BOARD_SIZE][BOARD_SIZE], int turn, int possibleMoves[100][4], int* numMoves) {
	*numMoves = 0;

	// loop through the board and find all possible moves for the current player
	#pragma omp parallel for collapse(2)
	for (int fromRow = 0; fromRow < BOARD_SIZE; ++fromRow) {
		for (int fromCol = 0; fromCol < BOARD_SIZE; ++fromCol) {
			if (board[fromRow][fromCol] == turn || board[fromRow][fromCol] == turn + 2) {
				#pragma omp parallel for collapse(2)
				for (int toRow = 0; toRow < BOARD_SIZE; ++toRow) {
					for (int toCol = 0; toCol < BOARD_SIZE; ++toCol) {
						if (isValidMove(board, turn, fromRow, fromCol, toRow, toCol)) {
							// add the move to the list of possible moves
							possibleMoves[*numMoves][0] = fromRow;
							possibleMoves[*numMoves][1] = fromCol;
							possibleMoves[*numMoves][2] = toRow;
							possibleMoves[*numMoves][3] = toCol;
							(*numMoves)++;
						}
					}
				}
			}
		}
	}
}

// evaluate the board position
int evaluatePosition(int board[BOARD_SIZE][BOARD_SIZE]) {
	int player1Pieces = 0, player2Pieces = 0;
	int player1Kings = 0, player2Kings = 0;

	int score = 0;

	// count pieces and calculate score
	for (int row = 0; row < BOARD_SIZE; row++) {
		for (int col = 0; col < BOARD_SIZE; col++) {
			// score for normal pieces
			if (board[row][col] == PLAYER2) {
				player2Pieces++;
				score += 100;
			} else if (board[row][col] == PLAYER1) {
				player1Pieces++;
				score -= 100;
				// more score for king pieces
			} else if (board[row][col] == PLAYER2 + 2) {
				player2Kings++;
				score += 300;
			} else if (board[row][col] == PLAYER1 + 2) {
				player1Kings++;
				score -= 300;
			}

			// higher score for controlling the center of the board (rows 3-4 and columns 2-5)
			// block me from going into the center
			if (row >= 3 && row <= 4 && col >= 2 && col <= 5) {
				if (board[row][col] == PLAYER2)
					score += 50;
				else if (board[row][col] == PLAYER1)
					score -= 50;
				else if (board[row][col] == PLAYER2 + 2)
					score += 100;
				else if (board[row][col] == PLAYER1 + 2)
					score -= 100;
			}
		}
	}

	// add score for having more pieces and kings
	// and make it capture pieces and kings
	score += 10 * (player2Pieces + player2Kings - player1Pieces - player1Kings);
	//score += player2Pieces - player1Pieces + (0.5 * player2Kings - 0.5 * player1Kings);

	return score;
}

// minimax with alpha-beta prunning
int minimax(int board[BOARD_SIZE][BOARD_SIZE], int maxDepth, int depth, int turn, int alpha, int beta) {
	// when max depth is reached, start evaluating the position
	if (depth == maxDepth) {
		return evaluatePosition(board);
	}

	// get the posible moves for this position
	int moves[100][4];
	int numMoves = 0;
	getPossibleMoves(board, turn, moves, &numMoves);
	
	// AI turn,  max the score
	if (turn == PLAYER2) { 
		int maxScore = -9999;

		// for each of the possible moves, call minimax again
		for (int i = 0; i < numMoves; i++) {
			int fromRow = moves[i][0], fromCol = moves[i][1];
			int toRow = moves[i][2], toCol = moves[i][3];
			
			int boardCopy[BOARD_SIZE][BOARD_SIZE];
			copyBoard(board, boardCopy);

			makeMove(boardCopy, turn, fromRow, fromCol, toRow, toCol);
			int score = minimax(boardCopy, maxDepth, depth + 1, PLAYER1, alpha, beta);

			if (score > maxScore)
				maxScore = score;

			if (maxScore > alpha)
				alpha = maxScore;

			// beta prunning
			if (beta <= alpha) break;
		}

		return maxScore;
		// my turn, min the score
	} else { 
		int minScore = 9999;

		// for each of the possible moves, call minimax again
		for (int i = 0; i < numMoves; i++) {
			int fromRow = moves[i][0], fromCol = moves[i][1];
			int toRow = moves[i][2], toCol = moves[i][3];
			
			int boardCopy[BOARD_SIZE][BOARD_SIZE];
			copyBoard(board, boardCopy);

			makeMove(boardCopy, turn, fromRow, fromCol, toRow, toCol);
			int score = minimax(boardCopy, maxDepth, depth + 1, PLAYER2, alpha, beta);

			if (score < minScore)
				minScore = score;

			if (minScore < beta)
				beta = minScore;

			if (beta <= alpha) break;
		}

		return minScore;
	}
}

// get the best move for the AI opponent
int getBestMoveForOpponent(int board[BOARD_SIZE][BOARD_SIZE], int turn, int maxDepth, int* fromRow, int* fromCol, int* toRow, int* toCol, int* score, int rank, int numProcesses) {
	// get move possible moves to pick
	int moves[100][4];
	int numMoves = 0;
	getPossibleMoves(board, turn, moves, &numMoves);

	int bestScore = -9999;
	int bestMoveIndex = -1;

	int movesPerProcess = numMoves / numProcesses;
	int remainder = numMoves % numProcesses;

	int startIndex = rank * movesPerProcess + (rank < remainder ? rank : remainder);
	int endIndex = startIndex + movesPerProcess + (rank < remainder ? 1 : 0);

	// call minimax and get the index of the best move
	#pragma omp parallel for
	for (int i = startIndex; i < endIndex; ++i) {
		int currentFromRow = moves[i][0];
		int currentFromCol = moves[i][1];
		int currentToRow = moves[i][2];
		int currentToCol = moves[i][3];

		int boardCopy[BOARD_SIZE][BOARD_SIZE];
		copyBoard(board, boardCopy);

		makeMove(boardCopy, turn, currentFromRow, currentFromCol, currentToRow, currentToCol);
		int score = minimax(boardCopy, maxDepth, 0, PLAYER1, -9999, 9999);

		#pragma omp critical
		{
			if (score > bestScore) {
				bestScore = score;
				bestMoveIndex = i;
			}
		}
	}

	// update variables
	*fromRow = moves[bestMoveIndex][0];
	*fromCol = moves[bestMoveIndex][1];
	*toRow = moves[bestMoveIndex][2];
	*toCol = moves[bestMoveIndex][3];
	*score = bestScore;
}