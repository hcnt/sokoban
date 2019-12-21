#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum direction { LEFT = 4, RIGHT = 6, UP = 8, DOWN = 2 } Direction;

typedef struct move {
    bool isUndoMove;
    char box;
    Direction direction;
} Move;

typedef struct position {
    int x;
    int y;
} Pos;

typedef struct stateStack {
    Pos positionBeforeMove;
    Move move;
    struct stateStack* lastState;
} StateStack;

typedef struct charVector {
    char* arr;
    int n;
} BoardRow;

typedef struct Char2DVector {
    BoardRow* arr;
    int n;
} Board;

void addCharToBoardRow(char x, BoardRow* row) {
    row->arr = realloc(row->arr, (row->n + 1) * sizeof(char));
    row->arr[row->n] = x;
    row->n++;
}
void addRowToBoard(BoardRow row, Board* board) {
    board->arr = realloc(board->arr, (board->n + 1) * sizeof(BoardRow));
    board->arr[board->n] = row;
    board->n++;
}

BoardRow getOneLineOfInput() {
    char currentChar = getchar();
    BoardRow row;
    row.n = 0;
    row.arr = NULL;
    while (currentChar != '\n') {
        addCharToBoardRow(currentChar, &row);
        currentChar = getchar();
    }
    return row;
}
Board getBoardFromInput() {
    Board board;
    board.n = 0;
    board.arr = NULL;
    BoardRow currentRow;
    char tmp;
    while (true) {
        tmp = getchar();
        if (tmp == '\n')
            return board;
        else
            ungetc(tmp, stdin);
        currentRow = getOneLineOfInput();
        addRowToBoard(currentRow, &board);
    }
    return board;
}

void printRow(BoardRow row) {
    for (int i = 0; i < row.n; i++) {
        printf("%c", row.arr[i]);
    }
}

void printBoard(Board board) {
    for (int i = 0; i < board.n; i++) {
        printRow(board.arr[i]);
        printf("\n");
    }
}

void killBoard(Board board) {
    for (int i = 0; i < board.n; i++) {
        free(board.arr[i].arr);
    }
    free(board.arr);
}

// TODO IMPLEMENT THESE FUNCTIONS
Move getMoveFromInput() {
    Move move;
    return move;
}
void undoMove(Board board, StateStack* stack) {}
bool makeMove(Board board, Move move) { return true; };
void addMoveToStack(Move move, StateStack* stack) {}

void play() {
    Board board = getBoardFromInput();
    StateStack* stack;
    printBoard(board);
    bool wasMoveSuccessful;
    Move move;
    do {
        move = getMoveFromInput();
        if (move.isUndoMove)
            undoMove(board, stack);
        else {
            wasMoveSuccessful = makeMove(board, move);
            if (wasMoveSuccessful)
                addMoveToStack(move, stack);
        }
        printBoard(board);
    } while (move.direction != 0);
}

int main() {
    Board board = getBoardFromInput();
    printBoard(board);
    killBoard(board);
    return 0;
}