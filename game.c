#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#include "include/game.h"
#include "include/minimax.h"

char tokens[] = {FIRST_TOKEN, SECOND_TOKEN};

void checkMalloc(void *item) {
    if (item == NULL) {
        fprintf(stderr, "error: out of memory\n");
		exit(EXIT_FAILURE);
    }
}

void clearScreen() {
    const char *CLEAR_SCREEN_ANSI = "\033[1;1H\033[2J";
    write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, strlen(CLEAR_SCREEN_ANSI));
    fflush(stdout);
}

struct game *initialiseGame() {
    struct game *newGame = malloc(sizeof(struct game));
    checkMalloc(newGame);
    newGame->turn = FIRST;
    newGame->moveCount = 0;
    newGame->board = malloc(sizeof(char *) * NUM_ROWS);
    for (int i = 0; i < NUM_ROWS; i++) {
        newGame->board[i] = malloc(sizeof(char) * NUM_COLS);
        checkMalloc(newGame->board[i]);
        for (int j = 0; j < NUM_COLS; j++) {
            newGame->board[i][j] = BLANK_TOKEN;
        }
    }
    return newGame;
}

void printBoard(struct game *game, char message[MAX_MESSAGE_LENGTH]) {
    clearScreen();
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            printf("%c ", game->board[i][j]);
        }
        printf("\n");
    }
    printf("%s\n", message);
}

bool hasWonHorizontal(struct game *game, bool turn) {
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            if (NUM_COLS - j < NUM_TO_WIN) break;

            char t0 = game->board[i][j];
            if (t0 != tokens[turn]) continue;

            char t1 = game->board[i][j + 1];
            char t2 = game->board[i][j + 2];
            char t3 = game->board[i][j + 3];
            if (t0 == t1 && t0 == t2 && t0 == t3) {
                return true;
            }
        }
    }
    return false;
}

bool hasWonVertical(struct game *game, bool turn) {
    for (int j = 0; j < NUM_COLS; j++) {
        for (int i = 0; i < NUM_ROWS; i++) {
            if (NUM_ROWS - i < NUM_TO_WIN) break;

            char t0 = game->board[i][j];
            if (t0 != tokens[turn]) continue;

            char t1 = game->board[i + 1][j];
            char t2 = game->board[i + 2][j];
            char t3 = game->board[i + 3][j];
            if (t0 == t1 && t0 == t2 && t0 == t3) {
                return true;
            }
        }
    }
    return false;
}

bool hasWonDiagonalTLBR(struct game *game, bool turn) {
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            if (NUM_COLS - j < NUM_TO_WIN ||
                NUM_ROWS - i < NUM_TO_WIN) break;

            char t0 = game->board[i][j];
            if (t0 != tokens[turn]) continue;

            char t1 = game->board[i + 1][j + 1];
            char t2 = game->board[i + 2][j + 2];
            char t3 = game->board[i + 3][j + 3];
            if (t0 == t1 && t0 == t2 && t0 == t3) {
                return true;
            }
        }
    }
    return false;
}

bool hasWonDiagonalBLTR(struct game *game, bool turn) {
    for (int i = NUM_ROWS - 1; i >= 0; i--) {
        for (int j = 0; j < NUM_COLS; j++) {
            if (NUM_COLS - j < NUM_TO_WIN ||
                i < NUM_TO_WIN - 1) break;
            
            char t0 = game->board[i][j];
            if (t0 != tokens[turn]) continue;

            char t1 = game->board[i - 1][j + 1];
            char t2 = game->board[i - 2][j + 2];
            char t3 = game->board[i - 3][j + 3];
            if (t0 == t1 && t0 == t2 && t0 == t3) {
                return true;
            }
        }
    }
    return false;
}

bool hasWon(struct game *game, bool turn) {
    return (hasWonHorizontal(game, turn) ||
            hasWonVertical(game, turn) ||
            hasWonDiagonalTLBR(game, turn) ||
            hasWonDiagonalBLTR(game, turn));
}

void placeTile(struct game *game, int playerInput, int insertRow) {
    if (insertRow < 0) return;

    if (game->board[insertRow][playerInput] != BLANK_TOKEN) {
        placeTile(game, playerInput, insertRow - 1);
    } else {
        game->board[insertRow][playerInput] = tokens[game->turn];
        game->moveStack[game->moveCount] = playerInput;
        game->moveCount++;
        game->turn = !game->turn;
    }
}

void undoMove(struct game *game) {
    if (game->moveCount > 0) {
        game->moveCount--;
        int column = game->moveStack[game->moveCount];
        for (int i = 0; i < NUM_ROWS; i++) {
            if (game->board[i][column] != BLANK_TOKEN) {
                game->board[i][column] = BLANK_TOKEN;
                game->turn = !game->turn;
                break;
            }
        }
    }
}

void gameLoop(struct game *game) {
    char message[MAX_MESSAGE_LENGTH];
    printBoard(game, message);
    char c;
    while ((c = getchar()) != EOF) {
        // message[0] = '\0';
        if (game->moveCount >= 42) {
            printf("It's a draw!\n");
        }
        if (c == 'q') {
            printf("Quitting!\n");
            break;
        }
        if (c == 'u') {
            undoMove(game);
            continue;
        }
        if (c == 'h') {
            struct node *gameTree = createTree(game, 6);
            struct move bestMove = minimax(gameTree, true);
            sprintf(message, "The best move for %c is to play column %d (evaluation of %d)", tokens[game->turn], bestMove.column + 1, bestMove.value);
        }

        c = c - '0' - 1;
        if (c < 0 || c > NUM_COLS - 1) {
            printBoard(game, message);
            continue;
        }

        placeTile(game, c, NUM_ROWS - 1);
        if (hasWon(game, !game->turn)) {
            printBoard(game, message);
            printf("%c has won!\n", tokens[!game->turn]);
            break;
        }
        printBoard(game, message);
    }
    printf("Game ended!\n");
    for (int i = 0; i < NUM_ROWS; i++) {
        free(game->board[i]);
    }
    free(game->board);
}

int main() {
    struct game *game = initialiseGame();
    gameLoop(game);
}
