/*
 * Noughts and Crosses
 *
 * Compile with
 *     c++ noughts_and_crosses.cpp -o
 *         noughts_and_crosses.o
 *         -std=c++11
 * Run with
 *     ./noughts_and_crosses
 *
 * Enjoy!
 */

#include <iostream>

// Marker types.
enum marker {
	mk_none,
	mk_nought,
	mk_cross
};

// Marker characters.
char marker_char[] = {
	' ', 'O', 'X'
};

// Print a board.
void print_board(marker board[3][3]) {
	//    123 
	//   +---+
	// 1 |X0X|
	// 2 |OXO|
	// 3 |XOX|
	//   +---+
	std::cout << "   123 " << std::endl;
	std::cout << "  +---+" << std::endl;
	for (int i = 0; i < 3; i++) {
		std::cout << i + 1 << " |";
		for (int j = 0; j < 3; j++) {
			std::cout << marker_char[board[i][j]];
		}
		std::cout << "|" << std::endl;
	}
	std::cout << "  +---+" << std::endl;
}

// Check if a marker is a winner.
bool sub_winner(marker board[3][3], marker player) {
	// Check for horizontal lines.
	for (int i = 0; i < 3; i++) {
		int matches = 0;
		for (int j = 0; j < 3; j++) {
			matches += board[j][i] == player;
		}
		if (matches == 3) {
			return true;
		}
	}
	// Check for vertical lines.
	for (int i = 0; i < 3; i++) {
		int matches = 0;
		for (int j = 0; j < 3; j++) {
			matches += board[i][j] == player;
		}
		if (matches == 3) {
			return true;
		}
	}
	// Check for top-left to bottom-right diagonals.
	int matches_1 = 0;
	for (int i = 0; i < 3; i++) {
		matches_1 += board[i][i] == player;
	}
	if (matches_1 == 3) {
		return true;
	}
	// Check for bottom-left to top-right diagonals.
	int matches_2 = 0;
	for (int i = 0; i < 3; i++) {
		matches_2 += board[i][2 - i] == player;
	}
	if (matches_2 == 3) {
		return true;
	}
	// Player is not a winner.
	return false;
}

// Check for a winner.
bool winner(marker board[3][3]) {
	return sub_winner(board, mk_nought) ||
		   sub_winner(board, mk_cross);	
}

// Entry point.
int main() {
	// Print the banner.
	std::cout << "#######################" << std::endl;
	std::cout << "# Noughts and Crosses #" << std::endl;
	std::cout << "#######################" << std::endl;
	// Create the board.
	marker board[3][3];
	// Clear the board.
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			board[i][j] = mk_none;
		}
	}
	// Start the game.
	for (;;) {
		// Print the board.
		print_board(board);
		// Noughts go first.
		noughts:
		std::cout << "Noughts (O) turn! Say (x y)" << std::endl;
		// Read the coordinates.
		int x;
		int y;
		std::cin >> x;
		std::cin >> y;
		// Make sure the coordinates are valid.
		if (x < 1 || x > 3)
			goto noughts;
		if (y < 1 || y > 3)
			goto noughts;
		// Plot the marker.
		if (board[y - 1][x - 1] != mk_none) {
			std::cout << "That space is taken!" << std::endl;
			goto noughts;
		}
		board[y - 1][x - 1] = mk_nought;
		// Check for a winner.
		if (winner(board)) {
			print_board(board);
			std::cout << "****************" << std::endl;
			std::cout << "* NOUGHTS WIN! *" << std::endl;
			std::cout << "****************" << std::endl;
			break;
		}
		// Print the board.
		print_board(board);
		// Crosses go second.
		crosses:
		std::cout << "Crosses (O) turn! Say (x y)" << std::endl;
		// Read the coordinates.
		std::cin >> x;
		std::cin >> y;
		// Make sure the coordinates are valid.
		if (x < 1 || x > 3)
			goto noughts;
		if (y < 1 || y > 3)
			goto noughts;
		// Plot the marker.
		if (board[y - 1][x - 1] != mk_none) {
			std::cout << "That space is taken!" << std::endl;
			goto crosses;
		}
		board[y - 1][x - 1] = mk_cross;
		// Check for a winner.
		if (winner(board)) {
			print_board(board);
			std::cout << "****************" << std::endl;
			std::cout << "* CROSSES WIN! *" << std::endl;
			std::cout << "****************" << std::endl;
			break;
		}
	}
	// Somebody won, so exit.
	return 0;
}