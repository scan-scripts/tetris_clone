#include <assert.h>
// #include <math.h>
#include <stdio.h>

#include "raylib.h"
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
#define GAME_SCREEN_HEIGHT 800

/*
TODO
game over logic
points
levels
speed up with levels

menu
outlines for all cells (make a function for drawing a cell so we can change the style later)
show next 3 pieces (optional with menu)
level select in menu
different randomization
music
refactor so gamestate is global variable


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

void GetTetrominoGridPoints(Tetromino tetromino, Point *gridPoints) {
    for (int i = 0; i < 4; i++) {
        int x = TetrominoShapeTable[tetromino.type][tetromino.rotState][i].x + tetromino.x;
        int y = TetrominoShapeTable[tetromino.type][tetromino.rotState][i].y + tetromino.y;
        gridPoints[i] = (Point){x, y};
    }
}

void GetCurrentTetrominoGridPoints(GameState *p_gameState, Point *gridPoints) {
    Tetromino tetromino = p_gameState->current;
    GetTetrominoGridPoints(tetromino, gridPoints);
}

bool CanPlaceTetromino(GameState *p_gameState, Tetromino *p_tetromino) {
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

bool CanPlaceCurrentTetromino(GameState *p_gameState) { return CanPlaceTetromino(p_gameState, &p_gameState->current); }

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

void RotateCurrentTetromino(GameState *p_gameState) {
    RotationState oldRotation = p_gameState->current.rotState;
    int oldX = p_gameState->current.x;
    int oldY = p_gameState->current.y;

    p_gameState->current.rotState = CycleRotationState(oldRotation);

    for (int i = 0; i < 5; i++) {
        Point dxy = {0};

        if (p_gameState->current.type == I) {
            dxy = wallKickTranslationTable_I[oldRotation][i];
        } else {
            dxy = wallKickTranslationTable_JLSTZ[oldRotation][i];
        }

        p_gameState->current.x = oldX + dxy.x;
        p_gameState->current.y = oldY + dxy.y;

        if (CanPlaceCurrentTetromino(p_gameState)) {
            return;
        }
    }

    p_gameState->current.rotState = oldRotation;
    p_gameState->current.x = oldX;
    p_gameState->current.y = oldY;
}

bool MoveCurrentTetrominoSide(GameState *p_gameState, int dX) {
    p_gameState->current.x += dX;

    if (!CanPlaceCurrentTetromino(p_gameState)) {
        p_gameState->current.x -= dX;
        return false;
    }

    return true;
}

bool MoveCurrentTetrominoDown(GameState *p_gameState) {
    p_gameState->current.y += 1;

    if (!CanPlaceCurrentTetromino(p_gameState)) {
        p_gameState->current.y -= 1;
        return false;
    }

    return true;
}

void LockCurrentTetromino(GameState *p_gameState) {
    Point gridPoints[4] = {0};

    GetCurrentTetrominoGridPoints(p_gameState, gridPoints);

    for (int i = 0; i < 4; i++) {
        int x = gridPoints[i].x;
        int y = gridPoints[i].y;

        // assert(x >= 0 && x < BOARD_WIDTH);
        // assert(y >= 0 && y < BOARD_HEIGHT);
        // assert(p_gameState->board[y][x] == EMPTY);

        if (!(p_gameState->board[y][x] == EMPTY)) {
            p_gameState->gameOver = true;
        }
        p_gameState->board[y][x] = p_gameState->current.type;
    }
}

void RandomTetromino(Tetromino *p_tetromino) {
    p_tetromino->type = GetRandomValue(I, Z);
    p_tetromino->x = 4;
    p_tetromino->y = 0;
    p_tetromino->rotState = UP;
}

void SpawnNextTetromino(GameState *p_gameState) {
    p_gameState->current = p_gameState->next;
    p_gameState->current.x = 4;
    p_gameState->current.y = 0;

    RandomTetromino(&(p_gameState->next));
}

bool IsRowFull(GameState *p_gameState, int row) {
    for (int col = 0; col < BOARD_WIDTH; col++) {
        if (p_gameState->board[row][col] == EMPTY) {
            return false;
        }
    }

    return true;
}

void DropAboveRows(GameState *p_gameState, int clearedRow) {
    assert(clearedRow >= 0);
    assert(clearedRow < BOARD_HEIGHT);

    for (int row = clearedRow; row > 0; row--) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            p_gameState->board[row][col] = p_gameState->board[row - 1][col];
        }
    }

    for (int col = 0; col < BOARD_WIDTH; col++) {
        p_gameState->board[0][col] = EMPTY;
    }
}

int ClearLines(GameState *p_gameState) {
    int clearedLines = 0;
    for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
        if (IsRowFull(p_gameState, row)) {
            DropAboveRows(p_gameState, row);
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

void UpdateGameStats(GameState *p_gameState) {
    int clearedLines = ClearLines(p_gameState);
    p_gameState->linesCleared += clearedLines;
    float multiplier = 1.0f;
    if (clearedLines == 4) {
        if (p_gameState->streak) {
            multiplier = 1.4; // back to back factor check later
        }
        p_gameState->streak = true;
    } else {
        p_gameState->streak = false;
    }
    p_gameState->level = p_gameState->linesCleared / 10;
    p_gameState->score += PointsForClear(clearedLines) * multiplier * p_gameState->level;
}

void LockAndSpawnNextTetromino(GameState *p_gameState) {
    LockCurrentTetromino(p_gameState);
    UpdateGameStats(p_gameState);
    SpawnNextTetromino(p_gameState);
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

void DrawBoard(GameState *p_gameState) {
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

void DrawCurrentTetromino(GameState *p_gameState) { DrawTetromino(p_gameState->current); }

void StepCurrentTetrominoDown(GameState *p_gameState) {
    if (!MoveCurrentTetrominoDown(p_gameState)) {
        LockAndSpawnNextTetromino(p_gameState);
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

Tetromino GetShadowTetromino(GameState *p_gameState) {
    Tetromino shadow = p_gameState->current;

    while (true) {
        shadow.y++;

        if (!CanPlaceTetromino(p_gameState, &shadow)) {
            shadow.y--;
            break;
        }
    }

    return shadow;
}

void FastDrop(GameState *p_gameState) {
    Tetromino shadow = GetShadowTetromino(p_gameState);
    p_gameState->current = shadow;
    LockAndSpawnNextTetromino(p_gameState);
}

void HoldCurrent(GameState *p_gameState) {
    if (p_gameState->hold.type == EMPTY) {
        p_gameState->hold = p_gameState->current;
        SpawnNextTetromino(p_gameState);
    } else {
        Tetromino temp = p_gameState->hold;
        p_gameState->hold = p_gameState->current;
        p_gameState->current = temp;
        p_gameState->current.x = 4;
        p_gameState->current.y = 0;
        p_gameState->current.rotState = UP;
    }
}

void DrawCenteredTextInRec(const char *text, int fontSize, Color color, Rectangle rec) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, fontSize / 10.0f);

    int textX = rec.x + (rec.width / 2.0) - (textSize.x / 2.0);
    int textY = rec.y + (rec.height / 2.0) - (textSize.y / 2.0);
    DrawText(text, textX, textY, fontSize, color);
}

void ShowPauseMenu() {
    Rectangle rec = {0, PLAY_AREA_Y + PLAY_AREA_WIDTH / 2.0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT / 2.0};
    DrawRectangleRec(rec, BLUE);
    DrawCenteredTextInRec("PAUSE", 60, LIGHTGRAY, rec);
}

void ShowGameOverMenu() {
    Rectangle rec = {0, PLAY_AREA_Y + PLAY_AREA_WIDTH / 2.0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT / 2.0};
    DrawRectangleRec(rec, RED);
    DrawCenteredTextInRec(TextFormat("GAME OVER\nLines cleared: %i\nScore: %d\nlevel = %d", gameState.linesCleared,
                                     gameState.score, gameState.level),
                          60, LIGHTGRAY, rec);
}

void UpdateGame(float dT) {
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    }
    if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_P)) {
        gameState.pause = !gameState.pause;
    }
    if (gameState.pause) {
        return;
    }
    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_H)) {
        HoldCurrent(&gameState);
    }
    if (IsKeyPressed(KEY_UP)) {
        RotateCurrentTetromino(&gameState);
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
        MoveCurrentTetrominoSide(&gameState, dir);
        gameState.moveTimer = 0.0f;
    }

    // held movement
    if (dir != 0 && !IsKeyPressed(KEY_LEFT) && !IsKeyPressed(KEY_RIGHT)) {
        gameState.moveTimer += dT;

        if (gameState.moveTimer >= gameState.moveDelay) {
            MoveCurrentTetrominoSide(&gameState, dir);

            // switch to repeat interval after delay
            gameState.moveTimer = gameState.moveDelay - gameState.moveInterval;
        }
    } else {
        gameState.moveTimer = 0.0f;
    }

    if (IsKeyPressed(KEY_DOWN)) {
        StepCurrentTetrominoDown(&gameState);
        gameState.fallTimer = 0.0f;
        gameState.softDropTimer = 0.0f;
    }

    if (IsKeyDown(KEY_DOWN)) {
        gameState.softDropTimer += dT;

        if (gameState.softDropTimer >= gameState.softDropInterval) {
            StepCurrentTetrominoDown(&gameState);
            gameState.softDropTimer = 0.0f;
            gameState.fallTimer = 0.0f;
        }
    } else {
        gameState.softDropTimer = 0.0f;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        FastDrop(&gameState);
    }

    gameState.fallTimer += dT;

    // TODO move the clear lines call into update

    if (gameState.fallTimer >= gameState.fallInterval) {
        StepCurrentTetrominoDown(&gameState);
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

    for (int i = 0; i < 4; i++) {
        Point pos = TetrominoShapeTable[tetromino.type][UP][i];

        float cellX = x + (float)(pos.x - xMin) * CELL_WIDTH + xOffset;
        float cellY = y + (float)(pos.y - yMin) * CELL_HEIGHT + yOffset;

        DrawRectangle(cellX, cellY, CELL_WIDTH, CELL_HEIGHT, colors[tetromino.type]);
    }
}

void DrawNext(GameState *p_gameState) {
    DrawTetrominoInBox(p_gameState->next, PLAY_AREA_X + PLAY_AREA_WIDTH, PLAY_AREA_Y);
}

void DrawHold(GameState *p_gameState) {
    DrawTetrominoInBox(p_gameState->hold, PLAY_AREA_X - 5 * CELL_WIDTH, PLAY_AREA_Y);
}

void DrawGame(GameState *p_gameState) {
    ClearBackground(BLACK);
    DrawRectangle(0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_WIDTH, BLUE);
    DrawNext(p_gameState);
    DrawHold(p_gameState);

    DrawRectangle(PLAY_AREA_X, PLAY_AREA_Y, PLAY_AREA_WIDTH, PLAY_AREA_HEIGHT, DARKGRAY);
    DrawBackgroundGrid();
    DrawBoard(p_gameState);
    DrawCurrentTetromino(p_gameState);
    Tetromino shadow = GetShadowTetromino(p_gameState);
    DrawTetrominoShadow(shadow);
    DrawText(TextFormat("Lines cleared: %i\nScore: %d ", p_gameState->linesCleared, p_gameState->score), 0, 0, 20, RED);
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

    gameState.fallTimer = 0.0f;
    gameState.fallInterval = 0.5f;

    gameState.moveTimer = 0.0f;
    gameState.moveDelay = 0.15f;
    gameState.moveInterval = 0.05f;

    gameState.softDropTimer = 0.0f;
    gameState.softDropInterval = 0.05f;

    gameState.score = 0;
    gameState.linesCleared = 0;
    gameState.level = 1;

    gameState.gameOver = false;
    gameState.pause = false;
    gameState.streak = false;
}

int main(void) {
    SetRandomSeed(6969);

    const int initialWindowWidth = 900;
    const int initialWindowHeight = 900;

    const int gameScreenWidth = 800;
    const int gameScreenHeight = 800;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(initialWindowWidth, initialWindowHeight, "Tetris");
    InitGameState();

    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    SetTargetFPS(60);
    float dT;
    while (!WindowShouldClose()) {
        if (!gameState.gameOver) {
            dT = GetFrameTime();
            UpdateGame(&gameState, dT);
        }

        float scale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
        Rectangle source = {0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height};
        Rectangle dest = {(GetScreenWidth() - gameScreenWidth * scale) * 0.5f,
                          (GetScreenHeight() - gameScreenHeight * scale) * 0.5f, gameScreenWidth * scale,
                          gameScreenHeight * scale};

        BeginTextureMode(target);
        DrawGame(&gameState);
        if (gameState.gameOver) {
            ShowGameOverMenu();
        }
        if (gameState.pause) {
            ShowPauseMenu();
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
