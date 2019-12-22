#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum direction { LEFT = 4, RIGHT = 6, UP = 8, DOWN = 2 } Direction;

typedef struct move {
    bool isUndoMove;
    bool isTerminating;
    char box;
    Direction direction;
} Move;

typedef struct position {
    int x;
    int y;
} Position;

bool equals(Position a, Position b) { return a.x == b.x && a.y == b.y; }

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
    PositionList* pos = malloc(sizeof(PositionList));
    pos->pos = firstPos;
    pos->next = NULL;
    queue.firstItem = pos;
    queue.lastItem = pos;
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

typedef struct box {
    Position pos;
    char letter;
} Box;

typedef struct stateStack {
    Position positionBeforeMove;
    Move move;
    struct stateStack* lastState;
} StateStack;

typedef struct field {
    char x;
    bool wasVisited;
} Field;

typedef struct board {
    Field** arr;
    int xSize;
    int ySize;
    Position playerPos;
    Box* boxes;
    int nBoxes;
} Board;

char getCharAtPosition(Position position, Board board) {
    return board.arr[position.y][position.x].x;
}
void setCharAtPosition(char x, Position position, Board board) {
    board.arr[position.y][position.x].x = x;
}

void printBoard(Board board) {
    for (int i = 0; i < board.ySize; i++) {
        for (int j = 0; j < board.xSize; j++) {
            printf("%c", board.arr[i][j].x);
        }
        printf("\n");
    }
}

void killBoard(Board board) {
    for (int i = 0; i < board.xSize; i++) {
        free(board.arr[i]);
    }
    free(board.arr);
    free(board.boxes);
}
void addCharToArray(char x, Field** row, int* n) {
    *row = realloc(*row, (*n + 1) * sizeof(Field));
    (*row)[*n].x = x;
    (*row)[*n].wasVisited = false;
    (*n)++;
}
void addRowToBoard(Field* row, Board* board) {
    board->arr = realloc(board->arr, (board->ySize + 1) * sizeof(Field*));
    board->arr[board->ySize] = row;
    board->ySize++;
}
void addBoxToBoard(Box box, Board* board) {
    board->boxes = realloc(board->boxes, (board->nBoxes + 1) * sizeof(Box));
    board->boxes[board->nBoxes] = box;
    board->nBoxes++;
}

// box has to be in array
Position getBoxPosition(char box, Board board) {
    for (int i = 0; i < board.nBoxes; i++) {
        if (board.boxes[i].letter == box) {
            return board.boxes[i].pos;
        }
    }
    printf("error - box not found\n");
    return board.boxes[0].pos;
}
void setBoxPosition(char box, Position pos, Board board) {
    for (int i = 0; i < board.nBoxes; i++) {
        if (board.boxes[i].letter == box) {
            board.boxes[i].pos = pos;
        }
    }
    printf("box not found\n");
}

// TODO refactor
Field* getOneLineOfInput(Board* board, int* sizeToSet) {
    char currentChar = getchar();
    Field* row = NULL;
    int i = 0;
    Box newBox;
    Position pos;
    while (currentChar != '\n') {
        if (isalpha(currentChar)) {
            newBox.letter = currentChar;
            pos.x = i;
            pos.y = board->ySize;
            newBox.pos = pos;
            addBoxToBoard(newBox, board);
        }
        if (currentChar == '@' || currentChar == '*') {
            pos.x = i;
            pos.y = board->ySize;
            board->playerPos = pos;
        }
        addCharToArray(currentChar, &row, &i);
        currentChar = getchar();
    }
    *sizeToSet = i;
    return row;
}
Board getBoardFromInput() {
    Board board;
    board.xSize = 0;
    board.ySize = 0;
    board.arr = NULL;
    board.nBoxes = 0;
    board.boxes = NULL;
    Field* currentRow;
    char tmp;
    while (true) {
        tmp = getchar();
        if (tmp == '\n') {
            // TESTING
            // printf("nboxes: %d\n", board.nBoxes);
            // for (int i = 0; i < board.nBoxes; i++) {
            //     printf("box: %c, position: %d %d\n", board.boxes[i].letter,
            //            board.boxes[i].pos.x, board.boxes[i].pos.y);
            // }
            // // TESTING
            return board;
        } else
            ungetc(tmp, stdin);
        currentRow = getOneLineOfInput(&board, &(board.xSize));
        addRowToBoard(currentRow, &board);
        // printf("board ysize: %d\n", board.ySize);
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
    if (pos.x >= board.xSize || pos.x < 0 || pos.y < 0 || pos.y >= board.ySize)
        return false;
    // printf("%d %d\n", pos.x, pos.y);
    char target = getCharAtPosition(pos, board);
    if (target == '#' || isalpha(target)) {
        return false;
    }
    return true;
}

bool wasPositionVisited(Position pos, Board board) {
    return board.arr[pos.x][pos.y].wasVisited;
}
void setPositionVisited(Position pos, Board* board, bool x) {
    board->arr[pos.x][pos.y].wasVisited = x;
}

void resetAllPositions(Board* board) {
    Position tmp;
    for (int i = 0; i < board->ySize; i++) {
        for (int j = 0; j < board->xSize; j++) {
            tmp.x = i;
            tmp.y = j;
            setPositionVisited(tmp, board, false);
        }
    }
}
void printQueue(PositionQueue q) {
    PositionList* list = q.firstItem;
    printf("queue state: ");
    while (list != NULL) {
        printf("%d,%d ", list->pos.x, list->pos.y);
        list = list->next;
    }
    printf("\n");
}

bool isMovePossible(Position start, Position target, Board* board) {
    // printf("-------CHECKING IF MOVE POSSIBLE------\n");
    // printf("starting position: %d %d\n", start.x, start.y);
    // printf("target position: %d %d\n", target.x, target.y);
    PositionQueue queue = initQueue(start);
    Position pos;
    Position tmp;
    while (!isEmpty(queue)) {
        // printQueue(queue);
        pos = pop(&queue);
        // printf("checking position %d,%d\n", pos.x, pos.y);
        if (equals(pos, target)) {
            // printf("-------MOVE POSSIBLE------\n");
            resetAllPositions(board);
            return true;
        }
        for (int i = 2; i <= 8; i += 2) {
            tmp = getPositionAfterMove(pos, i, false);
            if (isNewPositionLegal(tmp, *board) &&
                !wasPositionVisited(tmp, *board)) {
                push(tmp, &queue);
                setPositionVisited(tmp, board, true);
            }
        }
    }
    resetAllPositions(board);
    // printf("-------MOVE NOT POSSIBLE------\n");
    return false;
}

Position makeMove(Board* board, Move move) {
    // printf("-------MAKING MOVE------\n");
    Position boxPosition = getBoxPosition(move.box, *board);
    // printf("current box position: %d, %d\n", boxPosition.x, boxPosition.y);
    Position targetPosition =
        getPositionAfterMove(boxPosition, move.direction, false);
    // printf("target box position: %d, %d\n", targetPosition.x,
    // targetPosition.y);

    if (targetPosition.x >= board->xSize || targetPosition.x < 0 ||
        targetPosition.y < 0 || targetPosition.y >= board->ySize) {
        // printf("-------MOVE ABORTED------\n");
        return board->playerPos;
    }

    char target = getCharAtPosition(targetPosition, *board);
    if (target == '#' || isalpha(target)) {
        // printf("invalid target: %c\n", target);
        // printf("-------MOVE ABORTED------\n");
        return board->playerPos;
    }
    Position playerTargetPosition =
        getPositionAfterMove(boxPosition, move.direction, true);
    // printf("target player position: %d, %d\n", playerTargetPosition.x,
    //    playerTargetPosition.y);

    if (isMovePossible(board->playerPos, playerTargetPosition, board)) {
        if (getCharAtPosition(board->playerPos, *board) == '@')
            setCharAtPosition('-', board->playerPos, *board);
        else
            setCharAtPosition('+', board->playerPos, *board);

        if (isupper(getCharAtPosition(boxPosition, *board))) {
            setCharAtPosition('*', boxPosition, *board);
        } else {
            setCharAtPosition('@', boxPosition, *board);
        }

        if (getCharAtPosition(targetPosition, *board) == '-')
            setCharAtPosition(move.box, targetPosition, *board);
        else
            setCharAtPosition(toupper(move.box), targetPosition, *board);

        board->playerPos = boxPosition;
        setBoxPosition(move.box, targetPosition, *board);
        // printf("-------MOVE MADE------\n");
    } else {
        // printf("-------MOVE NOT MADE------\n");
    }
    return board->playerPos;
}
void printMoveStack(StateStack* stack) {
    printf("state stack:\n");
    while (stack != NULL) {
        printf("move: %c%d, playerPos:%d %d\n", stack->move.box,
               stack->move.direction, stack->positionBeforeMove.x,
               stack->positionBeforeMove.y);
        stack = stack->lastState;
    }
}

void undoMove(Board* board, StateStack** stack) {
    // printf("-------UNDOING MOVE------\n");
    // printMoveStack(*stack);
    if (stack == NULL)
        return;
    // move box to the current player position
    char box = (*stack)->move.box;
    Position currentBoxPosition = getBoxPosition(box, *board);
    // printf("move box from %d %d to %d %d\n", );
    if (getCharAtPosition(board->playerPos, *board) == '@')
        setCharAtPosition(box, board->playerPos, *board);
    else
        setCharAtPosition(toupper(box), board->playerPos, *board);

    if (isupper(getCharAtPosition(currentBoxPosition, *board)))
        setCharAtPosition('+', currentBoxPosition, *board);
    else
        setCharAtPosition('-', currentBoxPosition, *board);

    setBoxPosition((*stack)->move.box, board->playerPos, *board);

    // move player to the position before move
    board->playerPos = (*stack)->positionBeforeMove;
    if (getCharAtPosition(board->playerPos, *board) == '-')
        setCharAtPosition('@', board->playerPos, *board);
    else
        setCharAtPosition('*', board->playerPos, *board);

    // pop state from a stack
    StateStack* tmp = *stack;
    (*stack) = (*stack)->lastState;
    free(tmp);
    // printf("-------UNDOING MOVE COMPLETE------\n");
}
void addStateToStack(Move move, Position playerPos, StateStack** stack) {
    StateStack* tmp = malloc(sizeof(StateStack));
    tmp->lastState = *stack;
    tmp->move = move;
    tmp->positionBeforeMove = playerPos;
    *stack = tmp;
}

void play() {
    // printf("board size: %d %d\n", board.xSize, board.ySize);
    Board board = getBoardFromInput();
    Position playerPositionBeforeMove = board.playerPos;
    Position playerPositionAfterMove;
    Move move;
    StateStack* stack = NULL;
    printBoard(board);
    while (true) {
        move = getMoveFromInput();
        getchar();
        if (move.isUndoMove) {
            // printf("undo move\n");
            undoMove(&board, &stack);
        } else if (move.isTerminating) {
            return;
        } else {
            playerPositionAfterMove = makeMove(&board, move);
            // printf("player position after move: %d, %d\n",
            //    playerPositionAfterMove.x, playerPositionAfterMove.y);
            if (!equals(playerPositionAfterMove, playerPositionBeforeMove)) {
                addStateToStack(move, playerPositionBeforeMove, &stack);
                // printMoveStack(stack);
            }
        }
        playerPositionBeforeMove = playerPositionAfterMove;
        printBoard(board);
    }
    killBoard(board);
}

int main() {
    play();
    return 0;
}