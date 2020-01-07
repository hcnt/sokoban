#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//------MOVE----------
typedef enum direction { LEFT = 4, RIGHT = 6, UP = 8, DOWN = 2 } Direction;
typedef struct move {
    bool isUndoMove;
    bool isTerminating;
    char box;
    Direction direction;
} Move;
//------------------------------------

//----------POSITION-------------
typedef struct position {
    int x;
    int y;
} Position;

bool equals(Position a, Position b) { return a.x == b.x && a.y == b.y; }
//------------------------------------

//-------POSITION QUEUE------------------
typedef struct positionList {
    Position pos;
    struct positionList* next;
} PositionList;

typedef struct positionQueue {
    PositionList* firstItem;
    PositionList* lastItem;
} PositionQueue;

PositionQueue initQueue(Position firstPos) {
    PositionQueue queue;
    PositionList* posList = malloc(sizeof(PositionList));
    posList->pos = firstPos;
    posList->next = NULL;
    queue.firstItem = posList;
    queue.lastItem = posList;
    return queue;
}
void push(Position pos, PositionQueue* queue) {
    PositionList* tmp = malloc(sizeof(PositionList));
    tmp->next = NULL;
    tmp->pos = pos;
    if (queue->firstItem == NULL) {
        queue->lastItem = tmp;
        queue->firstItem = queue->lastItem;
    } else {
        queue->lastItem->next = tmp;
        queue->lastItem = queue->lastItem->next;
    }
}
Position pop(PositionQueue* queue) {
    PositionList* tmp = queue->firstItem;
    queue->firstItem = queue->firstItem->next;
    Position pos = tmp->pos;
    free(tmp);
    return pos;
}
bool isEmpty(PositionQueue queue) { return queue.firstItem == NULL; }

void clearQueue(PositionQueue* queue) {
    while (!isEmpty(*queue)) {
        pop(queue);
    }
}
//------------------------------------

//----------STATE STACK-------------
typedef struct stateStack {
    Position positionBeforeMove;
    Move move;
    struct stateStack* lastState;
} StateStack;

void addStateToStack(Move move, Position playerPos, StateStack** stack) {
    StateStack* tmp = malloc(sizeof(StateStack));
    tmp->lastState = *stack;
    tmp->move = move;
    tmp->positionBeforeMove = playerPos;
    *stack = tmp;
}
StateStack popStack(StateStack** stack) {
    StateStack* tmp = *stack;
    StateStack value = *tmp;
    (*stack) = (*stack)->lastState;
    free(tmp);
    return value;
}
void clearStack(StateStack** stack) {
    while (*stack != NULL) {
        popStack(stack);
    }
}
//--------------------------------

//-----------BOARD-------------------
typedef struct field {
    char x;
    bool wasVisited;
} Field;

typedef struct box {
    Position pos;
    char letter;
} Box;

typedef struct row {
    Field* arr;
    int n;
} Row;

typedef struct board {
    Row* arr;
    int n;
    Box* boxes;
    int nBoxes;
    Position playerPos;
} Board;

char getCharAtPosition(Position position, Board board) {
    return board.arr[position.y].arr[position.x].x;
}
void setCharAtPosition(char x, Position position, Board board) {
    board.arr[position.y].arr[position.x].x = x;
}
void printBoard(Board board) {
    for (int i = 0; i < board.n; i++) {
        for (int j = 0; j < board.arr[i].n; j++) {
            printf("%c", board.arr[i].arr[j].x);
        }
        printf("\n");
    }
}
void clearBoard(Board board) {
    for (int i = 0; i < board.n; i++) {
        free(board.arr[i].arr);
    }
    free(board.arr);
    free(board.boxes);
}
void addCharToRow(char x, Row* row) {
    row->arr = realloc(row->arr, (row->n + 1) * sizeof(Field));
    row->arr[row->n].x = x;
    row->arr[row->n].wasVisited = false;
    row->n++;
}
void addRowToBoard(Row row, Board* board) {
    board->arr = realloc(board->arr, (board->n + 1) * sizeof(Row));
    board->arr[board->n] = row;
    board->n++;
}
void addBoxToBoard(Box box, Board* board) {
    board->boxes = realloc(board->boxes, (board->nBoxes + 1) * sizeof(Box));
    board->boxes[board->nBoxes] = box;
    board->nBoxes++;
}
Position getBoxPosition(char box, Board board) {
    box = tolower(box);
    for (int i = 0; i < board.nBoxes; i++) {
        if (board.boxes[i].letter == box) {
            return board.boxes[i].pos;
        }
    }
    printf("error - box not found\n");
    return (Position){-1, -1};
}
void setBoxPosition(char box, Position pos, Board board) {
    box = tolower(box);
    for (int i = 0; i < board.nBoxes; i++) {
        if (board.boxes[i].letter == box) {
            board.boxes[i].pos = pos;
            return;
        }
    }
    printf("error - box not found\n");
}
//------------------------------------

//--------INPUT------------
Row getOneLineOfInput(Board* board) {
    char currentChar = getchar();
    Row row;
    row.arr = NULL;
    row.n = 0;
    Box newBox;
    while (currentChar != '\n') {
        if (isalpha(currentChar)) {
            newBox.letter = tolower(currentChar);
            newBox.pos = (Position){row.n, board->n};
            addBoxToBoard(newBox, board);
        }
        if (currentChar == '@' || currentChar == '*') {
            board->playerPos = (Position){row.n, board->n};
        }
        addCharToRow(currentChar, &row);
        currentChar = getchar();
    }
    return row;
}
Board getBoardFromInput() {
    Board board;
    board.n = 0;
    board.nBoxes = 0;
    board.arr = NULL;
    board.boxes = NULL;
    Row currentRow;
    char tmp;
    while (true) {
        tmp = getchar();
        if (tmp == '\n') {
            return board;
        } else
            ungetc(tmp, stdin);
        currentRow = getOneLineOfInput(&board);
        addRowToBoard(currentRow, &board);
    }
    return board;
}
Move getMoveFromInput() {
    Move move;
    move.isUndoMove = false;
    move.isTerminating = false;
    char tmp;
    tmp = getchar();
    if (tmp == '.') {
        move.isTerminating = true;
        return move;
    }
    if (tmp == '0') {
        move.isUndoMove = true;
        return move;
    }
    move.box = tmp;
    tmp = getchar();
    move.direction = tmp - '0';
    return move;
}
//---------------------------

Position getPositionAfterMove(Position start, Direction dir, bool reverseMove) {
    int r = reverseMove ? -1 : 1;
    switch (dir) {
    case LEFT:
        start.x -= 1 * r;
        break;
    case RIGHT:
        start.x += 1 * r;
        break;
    case UP:
        start.y -= 1 * r;
        break;
    case DOWN:
        start.y += 1 * r;
        break;
    }
    return start;
}

bool isNewPositionLegal(Position pos, Board board) {
    if (pos.y >= board.n || pos.y < 0 || pos.x >= board.arr[pos.y].n ||
        pos.x < 0)
        return false;
    char target = getCharAtPosition(pos, board);
    if (target == '#' || isalpha(target)) {
        return false;
    }
    return true;
}
bool wasPositionVisited(Position pos, Board board) {
    return board.arr[pos.y].arr[pos.x].wasVisited;
}
void setPositionVisited(Position pos, Board* board, bool x) {
    board->arr[pos.y].arr[pos.x].wasVisited = x;
}
void resetAllPositions(Board* board) {
    for (int i = 0; i < board->n; i++) {
        for (int j = 0; j < board->arr[i].n; j++) {
            setPositionVisited((Position){j, i}, board, false);
        }
    }
}

bool isMovePossible(Position start, Position target, Board* board) {
    PositionQueue queue = initQueue(start);
    Position pos;
    Position tmp;
    while (!isEmpty(queue)) {
        pos = pop(&queue);
        if (equals(pos, target)) {
            clearQueue(&queue);
            resetAllPositions(board);
            return true;
        }
        for (int i = 2; i <= 8; i += 2) {
            tmp = getPositionAfterMove(pos, i, false);
            if (isNewPositionLegal(tmp, *board) &&
                !wasPositionVisited(tmp, *board)) {
                setPositionVisited(tmp, board, true);
                push(tmp, &queue);
            }
        }
    }
    clearQueue(&queue);
    resetAllPositions(board);
    return false;
}

// set char at position to '+' or '-', returns removed char
char clearBoardPosition(Position pos, Board* board) {
    char x = getCharAtPosition(pos, *board);
    char tmp;
    if (x == '@' || islower(x))
        tmp = '-';
    else
        tmp = '+';
    setCharAtPosition(tmp, pos, *board);
    return x;
}

void applyMoveToBoard(Position boxPosition, Position boxPositionAfterMove,
                      Position playerPosition, Position playerPositionAfterMove,
                      Board* board) {
    char tmpCurrent;
    char tmpAfter;
    // Remove player and box symbols from current positions
    clearBoardPosition(playerPosition, board);
    char boxSymbol = clearBoardPosition(boxPosition, board);

    // add player and box symbols to positions after move
    tmpCurrent = getCharAtPosition(playerPositionAfterMove, *board);
    if (tmpCurrent == '-')
        tmpAfter = '@';
    else
        tmpAfter = '*';
    setCharAtPosition(tmpAfter, playerPositionAfterMove, *board);

    tmpCurrent = getCharAtPosition(boxPositionAfterMove, *board);
    if (tmpCurrent == '-')
        tmpAfter = tolower(boxSymbol);
    else
        tmpAfter = toupper(boxSymbol);
    setCharAtPosition(tmpAfter, boxPositionAfterMove, *board);

    board->playerPos = playerPositionAfterMove;
    setBoxPosition(boxSymbol, boxPositionAfterMove, *board);
}

void makeMove(Board* board, Move move) {
    Position boxPosition = getBoxPosition(move.box, *board);
    Position targetPosition =
        getPositionAfterMove(boxPosition, move.direction, false);

    if (!isNewPositionLegal(targetPosition, *board))
        return;

    Position playerTargetPosition =
        getPositionAfterMove(boxPosition, move.direction, true);

    if (isMovePossible(board->playerPos, playerTargetPosition, board))
        applyMoveToBoard(boxPosition, targetPosition, board->playerPos,
                         boxPosition, board);
}

void undoMove(Board* board, StateStack** stack) {
    if (*stack == NULL)
        return;
    StateStack state = popStack(stack);
    Position currentBoxPosition = getBoxPosition(state.move.box, *board);
    applyMoveToBoard(currentBoxPosition, board->playerPos, board->playerPos,
                     state.positionBeforeMove, board);
}

void play() {
    Board board = getBoardFromInput();
    Position playerPositionBeforeMove = board.playerPos;
    Move move;
    StateStack* stack = NULL;
    printBoard(board);
    while (true) {
        move = getMoveFromInput();
        getchar(); // skip endline char
        if (move.isTerminating) {
            clearStack(&stack);
            clearBoard(board);
            return;
        } else if (move.isUndoMove) {
            undoMove(&board, &stack);
            playerPositionBeforeMove = board.playerPos;
        } else {
            makeMove(&board, move);
            if (!equals(board.playerPos, playerPositionBeforeMove))
                addStateToStack(move, playerPositionBeforeMove, &stack);
            playerPositionBeforeMove = board.playerPos;
        }
        printBoard(board);
    }
}

int main() {
    play();
    return 0;
}