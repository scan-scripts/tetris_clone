#ifndef TYPES_H
#define TYPES_H



#include <stdbool.h>
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20



typedef struct Point_t{
    int x; 
    int y;
}Point;



typedef enum TetrominoType_t {
    EMPTY,
    I,
    O,
    T,
    L,
    J,
    S,
    Z
}TetrominoType;


typedef enum RotationState_t {
    RIGHT,
    DOWN,    
    LEFT,
    UP
}RotationState;



 
typedef struct Tetromino_t{
   TetrominoType type; 
   RotationState rotState;     
   int x;
   int y;
}Tetromino;  

typedef struct GameState_t {
   TetrominoType board[BOARD_HEIGHT][BOARD_WIDTH];
   Tetromino current;
   Tetromino next; 
   Tetromino held;
   float fallTimer;
   float fallInterval;
   int score; 
   int linesCleared; 
   bool gameOver;   
}GameState;


typedef struct PlayArea_t{
    int x;
    int y; 
    int width; 
    int height;
}PlayArea;






#endif
