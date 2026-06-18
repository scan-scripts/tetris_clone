#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "scoreboard.h"
#include "tetris_menus.h"
#include "tetromino.h"
#include "types.h"
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define PLAY_AREA_WIDTH 300
#define PLAY_AREA_HEIGHT 600
#define CELL_HEIGHT PLAY_AREA_HEIGHT / BOARD_HEIGHT
#define CELL_WIDTH PLAY_AREA_WIDTH / BOARD_WIDTH
#define PLAY_AREA_X 8 * CELL_WIDTH
#define PLAY_AREA_Y 100

#define GAME_SCREEN_WIDTH 800
#define GAME_SCREEN_WIDTH 800
#define GAME_SCREEN_HEIGHT 800

/*
TODO
menus show next 3 pieces (optional with menu) level select in menu different randomization music refactor so
fix lock delay so it still gives you some time even if you have not moved the piece recently
setup consistant menu system for settings and the main menu. settings would then branch into things like audo menu and
game modes and level select


*/

Color colors[8] = {
    BLACK,   // EMPTY
    SKYBLUE, // I
    RED,     // O
    ORANGE,  // T
    YELLOW,  // L
    GREEN,   // J
    PURPLE,  // S
    BLUE     // Z
};

// GLOBAL GAME STATE
GameState gameState = {0};

void StartGame(void) {
    InitGameState();
    gameState.mainMenu = false;
}
void QuitGame(void) { CloseWindow(); }
void ResumeFromPause(void) {
    gameState.pause = false;
    MenuStackPop(&gameState.menuStack);
}
void GoToMainMenu(void) {
    gameState.pause = false;
    gameState.mainMenu = true;
    gameState.menuStack.stack[0] = &mainMenu;
    gameState.menuStack.depth = 0;
}

void GetTetrominoGridPoints(Tetromino tetromino, Point *gridPoints) {
    for (int i = 0; i < 4; i++) {
        int x = TetrominoShapeTable[tetromino.type][tetromino.rotState][i].x + tetromino.x;
        int y = TetrominoShapeTable[tetromino.type][tetromino.rotState][i].y + tetromino.y;
        gridPoints[i] = (Point){x, y};
    }
}

void GetCurrentTetrominoGridPoints(Point *gridPoints) {
    Tetromino tetromino = gameState.current;
    GetTetrominoGridPoints(tetromino, gridPoints);
}

bool CanPlaceTetromino(Tetromino tetromino) {
    Point gridPoints[4] = {0};

    for (int i = 0; i < 4; i++) {

        int x = TetrominoShapeTable[tetromino.type][tetromino.rotState][i].x + tetromino.x;
        int y = TetrominoShapeTable[tetromino.type][tetromino.rotState][i].y + tetromino.y;

        gridPoints[i] = (Point){x, y};
    }

    for (int i = 0; i < 4; i++) {
        int x = gridPoints[i].x;
        int y = gridPoints[i].y;

        if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT) {
            return false;
        }

        if (gameState.board[y][x] != EMPTY) {
            return false;
        }
    }

    return true;
}

bool CanPlaceCurrentTetromino() { return CanPlaceTetromino(gameState.current); }

void rotateTetromino(Tetromino *p_tetromino) {
    if (p_tetromino->rotState < UP) {
        p_tetromino->rotState++;
    } else {
        p_tetromino->rotState = UP;
    }
}

RotationState CycleRotationState(RotationState rState) {
    if (rState < UP) {
        return rState + 1;
    } else {
        return RIGHT;
    }
}

Point wallKickTranslationTable_JLSTZ[4][5] = {[UP] = {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
                                              [RIGHT] = {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
                                              [DOWN] = {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
                                              [LEFT] = {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}};

Point wallKickTranslationTable_I[4][5] = {[UP] = {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},
                                          [RIGHT] = {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}},
                                          [DOWN] = {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},
                                          [LEFT] = {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}}};

bool RotateCurrentTetromino() {
    RotationState oldRotation = gameState.current.rotState;
    int oldX = gameState.current.x;
    int oldY = gameState.current.y;

    gameState.current.rotState = CycleRotationState(oldRotation);

    for (int i = 0; i < 5; i++) {
        Point dxy = {0};

        if (gameState.current.type == I) {
            dxy = wallKickTranslationTable_I[oldRotation][i];
        } else {
            dxy = wallKickTranslationTable_JLSTZ[oldRotation][i];
        }

        gameState.current.x = oldX + dxy.x;
        gameState.current.y = oldY + dxy.y;

        if (CanPlaceCurrentTetromino()) {
            return true;
        }
    }

    gameState.current.rotState = oldRotation;
    gameState.current.x = oldX;
    gameState.current.y = oldY;
    return false;
}

bool MoveCurrentTetrominoSide(int dX) {
    gameState.current.x += dX;

    if (!CanPlaceCurrentTetromino()) {
        gameState.current.x -= dX;
        return false;
    }

    return true;
}

bool MoveCurrentTetrominoDown() {
    gameState.current.y += 1;

    if (!CanPlaceCurrentTetromino()) {
        gameState.current.y -= 1;
        return false;
    }

    return true;
}

void LockCurrentTetromino() {
    Point gridPoints[4] = {0};

    GetCurrentTetrominoGridPoints(gridPoints);

    for (int i = 0; i < 4; i++) {
        int x = gridPoints[i].x;
        int y = gridPoints[i].y;

        if (!(gameState.board[y][x] == EMPTY)) {
            gameState.gameOver = true;
            return;
        }
        gameState.board[y][x] = gameState.current.type;
    }
}

void RandomTetromino(Tetromino *p_tetromino) {
    p_tetromino->type = GetRandomValue(I, Z);
    p_tetromino->x = 4;
    p_tetromino->y = 0;
    p_tetromino->rotState = UP;
}

void SwapTetrominos(Tetromino *t1, Tetromino *t2) {
    Tetromino temp = *t1;
    *t1 = *t2;
    *t2 = temp;
}

void ClearBoard() {
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            gameState.board[i][j] = 0;
        }
    }
}

void InitGameState() {

    ClearBoard();

    RandomTetromino(&gameState.current);
    RandomTetromino(&gameState.next);
    gameState.hold = (Tetromino){0};

    gameState.alreadyHeld = false;

    gameState.fallTimer = 0.0f;
    gameState.fallInterval = 1.0f;

    gameState.moveTimer = 0.0f;
    gameState.moveDelay = 0.15f;
    gameState.moveInterval = 0.05f;

    gameState.lockDelayTimer = 0.0f;
    gameState.lockDelay = 0.5;

    gameState.softDropTimer = 0.0f;

    gameState.score = 0;
    gameState.linesCleared = 0;
    gameState.level = 1;

    gameState.gameOver = false;
    gameState.pause = false;
    gameState.streak = false;
    gameState.scoreAdded = false;
    InitScoreBoard();
    gameState.menuStack.depth = -1;
}

void SpawnNextTetromino() {
    gameState.current = gameState.next;
    gameState.current.x = 4;
    gameState.current.y = 0;
    RandomTetromino(&gameState.next);
    gameState.alreadyHeld = false;
    gameState.lockDelayTimer = 0.0f;
}

bool IsRowFull(int row) {
    for (int col = 0; col < BOARD_WIDTH; col++) {
        if (gameState.board[row][col] == EMPTY) {
            return false;
        }
    }
    return true;
}

void DropAboveRows(int clearedRow) {
    assert(clearedRow >= 0);
    assert(clearedRow < BOARD_HEIGHT);

    for (int row = clearedRow; row > 0; row--) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            gameState.board[row][col] = gameState.board[row - 1][col];
        }
    }

    for (int col = 0; col < BOARD_WIDTH; col++) {
        gameState.board[0][col] = EMPTY;
    }
}

int ClearLines() {
    int clearedLines = 0;
    for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
        if (IsRowFull(row)) {
            DropAboveRows(row);
            clearedLines++;
            row++;
        }
    }

    return clearedLines;
}

int PointsForClear(int clearedLines) {
    switch (clearedLines) {
    case 1:
        return 100;
    case 2:
        return 300;
    case 3:
        return 500;
    case 4:
        return 800;
    }
    return 0;
}

float CalcFallInterval() { return powf((0.8 - ((float)(gameState.level - 1) * 0.007f)), gameState.level - 1); }

void UpdateStats() {
    int clearedLines = ClearLines();
    gameState.linesCleared += clearedLines;
    float multiplier = 1.0f;
    if (clearedLines == 4) {
        if (gameState.streak) {
            multiplier = 1.4; // back to back factor check later
        }
        gameState.streak = true;
    } else {
        gameState.streak = false;
    }
    gameState.level = 1 + gameState.linesCleared / 10;
    gameState.fallInterval = CalcFallInterval();
    gameState.lockDelay = gameState.fallInterval / 2;
    gameState.score += PointsForClear(clearedLines) * multiplier * gameState.level;
}

void LockAndSpawnNextTetromino() {
    LockCurrentTetromino();
    UpdateStats();
    SpawnNextTetromino();
}

void DrawSquare(Point pos, Color color) {
    DrawRectangle(PLAY_AREA_X + pos.x * CELL_WIDTH, PLAY_AREA_Y + pos.y * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, color);
}

void DrawShadowSquare(Point pos, Color color) {
    DrawRectangleLinesEx((Rectangle){(float)PLAY_AREA_X + pos.x * (float)CELL_WIDTH,
                                     (float)PLAY_AREA_Y + pos.y * (float)CELL_HEIGHT, (float)CELL_WIDTH,
                                     (float)CELL_HEIGHT},
                         4, color);
}

void DrawBoard() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            TetrominoType type = gameState.board[y][x];

            if (type == EMPTY) {
                continue;
            }

            DrawSquare((Point){x, y}, colors[type]);
            DrawShadowSquare((Point){x, y}, GRAY);
        }
    }
}

void DrawTetromino(Tetromino tetromino) {
    Point gridPoints[4] = {0};
    GetTetrominoGridPoints(tetromino, gridPoints);
    for (int i = 0; i < 4; i++) {
        DrawSquare(gridPoints[i], colors[tetromino.type]);
    }
}

void DrawTetrominoShadow(Tetromino tetromino) {
    Point gridPoints[4] = {0};
    GetTetrominoGridPoints(tetromino, gridPoints);
    for (int i = 0; i < 4; i++) {
        DrawShadowSquare(gridPoints[i], WHITE);
    }
}

void DrawCurrentTetromino() {
    DrawTetromino(gameState.current);
    DrawTetrominoShadow(gameState.current);
}

void StepCurrentTetrominoDown() {
    if (MoveCurrentTetrominoDown()) {
        gameState.lockDelayTimer = 0.0f;
    }
}

void DrawBackgroundGrid(void) {

    for (int i = 0; i <= BOARD_WIDTH; i++) {
        int lineX = PLAY_AREA_X + i * CELL_WIDTH;
        DrawLine(lineX, PLAY_AREA_Y, lineX, PLAY_AREA_Y + PLAY_AREA_HEIGHT, LIGHTGRAY);
    }

    for (int i = 0; i <= BOARD_HEIGHT; i++) {
        int lineY = PLAY_AREA_Y + i * CELL_HEIGHT;
        DrawLine(PLAY_AREA_X, lineY, PLAY_AREA_X + PLAY_AREA_WIDTH, lineY, LIGHTGRAY);
    }
}

Tetromino GetShadowTetromino() {
    Tetromino shadow = gameState.current;

    while (true) {
        shadow.y++;
        if (!CanPlaceTetromino(shadow)) {
            shadow.y--;
            break;
        }
    }

    return shadow;
}

void FastDrop() {
    Tetromino shadow = GetShadowTetromino();
    gameState.current = shadow;
    LockAndSpawnNextTetromino();
}

void HoldCurrent() {
    if (gameState.hold.type == EMPTY) {
        gameState.hold = gameState.current;
        SpawnNextTetromino();
    } else {
        if (!gameState.alreadyHeld) {

            Tetromino temp = gameState.hold;
            gameState.hold = gameState.current;
            gameState.current = temp;
            gameState.current.x = 4;
            gameState.current.y = 0;
            gameState.current.rotState = UP;
        }
    }
    gameState.alreadyHeld = true;
}

bool IsPieceGrounded() {
    Tetromino test = gameState.current;
    test.y += 1;

    return !CanPlaceTetromino(test);
}

void UpdateGame(float dT) {
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    }
    if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_P)) {
        gameState.pause = true;
        MenuStackPush(&gameState.menuStack, &pauseMenu);
        return;
    }
    if (gameState.pause) {
        return;
    }
    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_H)) {
        HoldCurrent();
    }
    if (IsKeyPressed(KEY_UP)) {
        if (RotateCurrentTetromino()) {
            gameState.lockDelayTimer = 0.0f;
        }
    }

    int dir = 0;

    if (IsKeyDown(KEY_LEFT)) {
        dir = -1;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        dir = 1;
    }

    // initial press = immediate move
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
        if (MoveCurrentTetrominoSide(dir)) {
            gameState.moveTimer = 0.0f;
            gameState.lockDelayTimer = 0.0f;
        }
    }

    // held movement
    if (dir != 0 && !IsKeyPressed(KEY_LEFT) && !IsKeyPressed(KEY_RIGHT)) {
        gameState.moveTimer += dT;

        if (gameState.moveTimer >= gameState.moveDelay) {
            if (MoveCurrentTetrominoSide(dir)) {
                gameState.lockDelayTimer = 0.0f;
            }

            // switch to repeat interval after delay
            gameState.moveTimer = gameState.moveDelay - gameState.moveInterval;
        }
    } else {
        gameState.moveTimer = 0.0f;
    }

    if (IsKeyPressed(KEY_DOWN)) {
        StepCurrentTetrominoDown();
        gameState.fallTimer = 0.0f;
        gameState.softDropTimer = 0.0f;
    }

    if (IsKeyDown(KEY_DOWN)) {
        gameState.softDropTimer += dT;

        if (gameState.softDropTimer >= gameState.fallInterval / 20.0) {
            StepCurrentTetrominoDown();
            gameState.softDropTimer = 0.0f;
            gameState.fallTimer = 0.0f;
        }
    } else {
        gameState.softDropTimer = 0.0f;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        FastDrop();
    }

    gameState.fallTimer += dT;

    if (IsPieceGrounded()) {
        gameState.lockDelayTimer += dT;

        if (gameState.lockDelayTimer >= gameState.lockDelay) {
            LockAndSpawnNextTetromino();
            return;
        }
    } else {
        gameState.lockDelayTimer = 0.0f;
    }

    if (gameState.fallTimer >= gameState.fallInterval) {
        StepCurrentTetrominoDown();
        gameState.fallTimer = 0.0f;
    }
}

// Rectangle
void DrawTetrominoInBox(Tetromino tetromino, int x, int y) {
    const float boxCellLength = 4.0f;
    const float boxWidth = boxCellLength * CELL_WIDTH;
    const float boxHeight = boxCellLength * CELL_HEIGHT;

    Rectangle rec = {x, y, boxWidth, boxHeight};
    DrawRectangleLinesEx(rec, 3.0f, GRAY);
    DrawRectangleRec(rec, BLACK);

    int xMin = TetrominoShapeTable[tetromino.type][UP][0].x;
    int xMax = xMin;
    int yMin = TetrominoShapeTable[tetromino.type][UP][0].y;
    int yMax = yMin;

    for (int i = 1; i < 4; i++) {
        Point pos = TetrominoShapeTable[tetromino.type][UP][i];

        if (pos.x < xMin)
            xMin = pos.x;
        if (pos.x > xMax)
            xMax = pos.x;
        if (pos.y < yMin)
            yMin = pos.y;
        if (pos.y > yMax)
            yMax = pos.y;
    }

    float shapeWidth = (float)(xMax - xMin + 1) * CELL_WIDTH;
    float shapeHeight = (float)(yMax - yMin + 1) * CELL_HEIGHT;

    float xOffset = (boxWidth - shapeWidth) / 2.0f;
    float yOffset = (boxHeight - shapeHeight) / 2.0f;
    if (tetromino.type != EMPTY) {
        for (int i = 0; i < 4; i++) {
            Point pos = TetrominoShapeTable[tetromino.type][UP][i];

            float cellX = x + (float)(pos.x - xMin) * CELL_WIDTH + xOffset;
            float cellY = y + (float)(pos.y - yMin) * CELL_HEIGHT + yOffset;

            DrawRectangle(cellX, cellY, CELL_WIDTH, CELL_HEIGHT, colors[tetromino.type]);
            DrawRectangleLinesEx((Rectangle){(float)cellX, (float)cellY, (float)CELL_WIDTH, (float)CELL_HEIGHT}, 4,
                                 WHITE);
        }
    }
}

void DrawNext() { DrawTetrominoInBox(gameState.next, PLAY_AREA_X + PLAY_AREA_WIDTH, PLAY_AREA_Y); }

void DrawHold() { DrawTetrominoInBox(gameState.hold, PLAY_AREA_X - 5 * CELL_WIDTH, PLAY_AREA_Y); }

void DrawStatsBox() {
    DrawText(TextFormat("Lines cleared: %i\nScore: %d, level: %d, FPS =%d ", gameState.linesCleared, gameState.score,
                        gameState.level, GetFPS()),
             0, 0, 20, RED);
}

void DrawGame() {
    ClearBackground(BLACK);
    DrawRectangle(0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_WIDTH, BLUE);
    DrawNext();
    DrawHold();

    DrawRectangle(PLAY_AREA_X, PLAY_AREA_Y, PLAY_AREA_WIDTH, PLAY_AREA_HEIGHT, DARKGRAY);
    DrawBackgroundGrid();
    DrawBoard();
    DrawCurrentTetromino();
    Tetromino shadow = GetShadowTetromino();
    DrawTetrominoShadow(shadow);
    DrawStatsBox();
}

int main(void) {
    SetRandomSeed(6969);

    const int initialWindowWidth = 1600;
    const int initialWindowHeight = 1600;

    const int gameScreenWidth = 800;
    const int gameScreenHeight = 800;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(initialWindowWidth, initialWindowHeight, "Tetris");

    InitGameState();
    gameState.mainMenu = true;
    gameState.menuStack.stack[0] = &mainMenu;
    gameState.menuStack.depth = 0;

    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    SetTargetFPS(60);
    float dT;
    while (!WindowShouldClose()) {
        dT = GetFrameTime();

        float scale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
        Rectangle source = {0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height};
        Rectangle dest = {(GetScreenWidth() - gameScreenWidth * scale) * 0.5f,
                          (GetScreenHeight() - gameScreenHeight * scale) * 0.5f, gameScreenWidth * scale,
                          gameScreenHeight * scale};

        BeginTextureMode(target);
        if (gameState.mainMenu) {
            ClearBackground(BLACK);
            UpdateMenu();
            Menu *currentMenu = MenuStackTop(&gameState.menuStack);
            if (currentMenu) {
                DrawRectangleRec((Rectangle){0, 0, gameScreenWidth, gameScreenHeight}, BLUE);
                DrawMenu(*currentMenu, (Rectangle){0, 0, gameScreenWidth, gameScreenHeight});
            }
        } else {
            if (!gameState.gameOver) {
                UpdateGame(dT);
            }
            DrawGame();
            if (gameState.gameOver) {
                if (!gameState.scoreAdded) {
                    gameState.scoreAdded = true;
                    int place = CheckHighScore(gameState.score);
                    if (place >= 0) {
                        PushNameEntryMenu(place);
                    } else {
                        PushGameOverMenu();
                    }
                }
                UpdateMenu();
                Menu *currentMenu = MenuStackTop(&gameState.menuStack);
                if (currentMenu) {
                    DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, (Color){0, 0, 0, 180});
                    DrawMenu(*currentMenu, (Rectangle){0, 0, gameScreenWidth, gameScreenHeight});
                }
            }
            if (gameState.pause) {
                UpdateMenu();
                Menu *currentMenu = MenuStackTop(&gameState.menuStack);
                if (currentMenu) {
                    DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, (Color){0, 0, 0, 180});
                    DrawMenu(*currentMenu, (Rectangle){0, 0, gameScreenWidth, gameScreenHeight});
                }
            }
        }
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
