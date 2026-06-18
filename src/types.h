#ifndef TYPES_H
#define TYPES_H

#include "menus.h"
#include <stdbool.h>
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define N_SCORE_ELS 6

typedef struct Point_t {
    int x;
    int y;
} Point;

typedef enum TetrominoType_t { EMPTY, I, O, T, L, J, S, Z } TetrominoType;

typedef enum RotationState_t { RIGHT, DOWN, LEFT, UP } RotationState;

typedef struct Tetromino_t {
    TetrominoType type;
    RotationState rotState;
    int x;
    int y;
} Tetromino;

typedef enum SoundStyle_t { CLASSIC, SILLY } SoundStyle;

typedef struct ScoreElement_t {
    char name[4];
    int score;
} ScoreElement;

typedef struct ScoreBoard_t {
    ScoreElement scores[N_SCORE_ELS];
} ScoreBoard;

typedef struct GameState_t {
    TetrominoType board[BOARD_HEIGHT][BOARD_WIDTH];
    Tetromino current;
    Tetromino next;
    Tetromino hold;

    bool alreadyHeld;

    float fallTimer;
    float fallInterval;

    float moveTimer;
    float moveDelay;
    float moveInterval;
    float lockDelay;
    float softDropTimer;
    float lockDelayTimer;
    int score;
    int linesCleared;
    int level;
    bool streak;

    bool gameOver;
    bool pause;
    bool mainMenu;
    bool scoreAdded;

    ScoreBoard scoreBoard;
    MenuStack menuStack;

    int musicVolume;
    int sfxVolume;
    SoundStyle musicStyle;
    SoundStyle sfxStyle;

    bool showShadow;
    bool allowHold;
    int nextCount;

} GameState;

#endif
