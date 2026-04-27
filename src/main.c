#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "raylib.h"
#include "types.h"
#include "tetromino.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

PlayArea playArea = {40, 40, 300, 600};

Color colors[8] = {
    BROWN,   // EMPTY
    SKYBLUE, // I
    RED,     // O
    ORANGE,  // T
    YELLOW,  // L
    GREEN,   // J
    PURPLE,  // S
    BLUE     // Z
};

void GetCurrentTetrominoGridPoints(GameState* p_gameState, Point* gridPoints) {
    Tetromino* p_tetromino = &p_gameState->current;

    for (int i = 0; i < 4; i++) {
        int x = TetrominoShapeTable[p_tetromino->type][p_tetromino->rotState][i].x + p_tetromino->x;
        int y = TetrominoShapeTable[p_tetromino->type][p_tetromino->rotState][i].y + p_tetromino->y;

        gridPoints[i] = (Point){x, y};
    }
}

bool CanPlaceTetromino(GameState* p_gameState, Tetromino* p_tetromino) {
    Point gridPoints[4] = {0};

    for (int i = 0; i < 4; i++) {
        int x = TetrominoShapeTable[p_tetromino->type][p_tetromino->rotState][i].x + p_tetromino->x;
        int y = TetrominoShapeTable[p_tetromino->type][p_tetromino->rotState][i].y + p_tetromino->y;

        gridPoints[i] = (Point){x, y};
    }

    for (int i = 0; i < 4; i++) {
        int x = gridPoints[i].x;
        int y = gridPoints[i].y;

        if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT) {
            return false;
        }

        if (p_gameState->board[y][x] != EMPTY) {
            return false;
        }
    }

    return true;
}

bool CanPlaceCurrentTetromino(GameState* p_gameState) {
    return CanPlaceTetromino(p_gameState, &p_gameState->current);
}

void RotateCurrentTetromino(GameState* p_gameState) {
    RotationState oldRotation = p_gameState->current.rotState;

    if (p_gameState->current.rotState < UP) {
        p_gameState->current.rotState++;
    } else {
        p_gameState->current.rotState = RIGHT;
    }

    if (!CanPlaceCurrentTetromino(p_gameState)) {
        p_gameState->current.rotState = oldRotation;
    }
}

bool MoveCurrentTetrominoSide(GameState* p_gameState, int dX) {
    p_gameState->current.x += dX;

    if (!CanPlaceCurrentTetromino(p_gameState)) {
        p_gameState->current.x -= dX;
        return false;
    }

    return true;
}

bool MoveCurrentTetrominoDown(GameState* p_gameState) {
    p_gameState->current.y += 1;

    if (!CanPlaceCurrentTetromino(p_gameState)) {
        p_gameState->current.y -= 1;
        return false;
    }

    return true;
}

void LockCurrentTetromino(GameState* p_gameState) {
    Point gridPoints[4] = {0};

    GetCurrentTetrominoGridPoints(p_gameState, gridPoints);

    for (int i = 0; i < 4; i++) {
        int x = gridPoints[i].x;
        int y = gridPoints[i].y;

        assert(x >= 0 && x < BOARD_WIDTH);
        assert(y >= 0 && y < BOARD_HEIGHT);
        assert(p_gameState->board[y][x] == EMPTY);

        p_gameState->board[y][x] = p_gameState->current.type;
    }
}


void RandomTetromino(Tetromino* p_tetromino){
    p_tetromino->type = GetRandomValue(I,Z);
    p_tetromino->x= 4;
    p_tetromino->y= 0;
    p_tetromino->rotState = RIGHT; 
    
}

void SpawnNextTetromino(GameState* p_gameState) {
    p_gameState->current = p_gameState->next;
    p_gameState->current.x = 4;
    p_gameState->current.y = 0;

    // Temporary fixed next piece. Later this should become random.
   RandomTetromino(&(p_gameState->next));
}

void StepCurrentTetrominoDown(GameState* p_gameState) {
    if (!MoveCurrentTetrominoDown(p_gameState)) {
        LockCurrentTetromino(p_gameState);
        SpawnNextTetromino(p_gameState);
    }
}

void DrawSquare(Point pos, Color color) {
    int x = playArea.x;
    int y = playArea.y;
    int width = playArea.width;
    int height = playArea.height;

    int cellHeight = height / BOARD_HEIGHT;
    int cellWidth = width / BOARD_WIDTH;

    DrawRectangle(
        x + pos.x * cellWidth,
        y + pos.y * cellHeight,
        cellWidth,
        cellHeight,
        color
    );
}

void DrawBoard(GameState* p_gameState) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            TetrominoType type = p_gameState->board[y][x];

            if (type == EMPTY) {
                continue;
            }

            DrawSquare((Point){x, y}, colors[type]);
        }
    }
}

void DrawCurrentTetromino(GameState* p_gameState) {
    Point gridPoints[4] = {0};

    GetCurrentTetrominoGridPoints(p_gameState, gridPoints);

    for (int i = 0; i < 4; i++) {
        DrawSquare(gridPoints[i], colors[p_gameState->current.type]);
    }
}

void DrawBackgroundGrid(void) {
    int x = playArea.x;
    int y = playArea.y;
    int width = playArea.width;
    int height = playArea.height;

    int cellHeight = height / BOARD_HEIGHT;
    int cellWidth = width / BOARD_WIDTH;

    for (int i = 0; i <= BOARD_WIDTH; i++) {
        int lineX = x + i * cellWidth;
        DrawLine(lineX, y, lineX, y + height, LIGHTGRAY);
    }

    for (int i = 0; i <= BOARD_HEIGHT; i++) {
        int lineY = y + i * cellHeight;
        DrawLine(x, lineY, x + width, lineY, LIGHTGRAY);
    }
}

void UpdateGame(GameState* p_gameState, float dT) {
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    }

    if (IsKeyPressed(KEY_UP)) {
        RotateCurrentTetromino(p_gameState);
    }

    if (IsKeyPressed(KEY_LEFT)) {
        MoveCurrentTetrominoSide(p_gameState, -1);
    }

    if (IsKeyPressed(KEY_RIGHT)) {
        MoveCurrentTetrominoSide(p_gameState, 1);
    }

    if (IsKeyPressed(KEY_DOWN)) {
        StepCurrentTetrominoDown(p_gameState);
        p_gameState->fallTimer = 0.0f;
    }

    p_gameState->fallTimer += dT;

    if (p_gameState->fallTimer >= p_gameState->fallInterval) {
        StepCurrentTetrominoDown(p_gameState);
        p_gameState->fallTimer = 0.0f;
    }
}

void DrawGame(GameState* p_gameState, int gameScreenWidth, int gameScreenHeight) {
    ClearBackground(BLACK);

    DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, BLUE);
    DrawRectangle(playArea.x, playArea.y, playArea.width, playArea.height, DARKGRAY);

    DrawBackgroundGrid();
    DrawBoard(p_gameState);
    DrawCurrentTetromino(p_gameState);
}

void InitGameState(GameState* p_gameState) {
    p_gameState->board[BOARD_HEIGHT - 1][0] = T;
    p_gameState->board[BOARD_HEIGHT - 1][1] = T;
    p_gameState->board[BOARD_HEIGHT - 1][2] = T;
    p_gameState->board[BOARD_HEIGHT - 2][1] = T;

    p_gameState->current = (Tetromino){L, UP, 6, 5};
    p_gameState->next = (Tetromino){S, LEFT, 4, 0};

    p_gameState->fallTimer = 0.0f;
    p_gameState->fallInterval = 0.5f;
}

int main(void) {
    SetRandomSeed(6969);


    const int initialWindowWidth = 900;
    const int initialWindowHeight = 900;

    const int gameScreenWidth = 800;
    const int gameScreenHeight = 800;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(initialWindowWidth, initialWindowHeight, "Tetris");

    GameState gameState = {0};
    InitGameState(&gameState);

    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float dT = GetFrameTime();

        UpdateGame(&gameState, dT);

        float scale = MIN(
            (float)GetScreenWidth() / gameScreenWidth,
            (float)GetScreenHeight() / gameScreenHeight
        );

        Rectangle source = {
            0.0f,
            0.0f,
            (float)target.texture.width,
            (float)-target.texture.height
        };

        Rectangle dest = {
            (GetScreenWidth() - gameScreenWidth * scale) * 0.5f,
            (GetScreenHeight() - gameScreenHeight * scale) * 0.5f,
            gameScreenWidth * scale,
            gameScreenHeight * scale
        };

        BeginTextureMode(target);
            DrawGame(&gameState, gameScreenWidth, gameScreenHeight);
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(target.texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}
