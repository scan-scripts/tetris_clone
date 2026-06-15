
#ifndef TETRIS_MENU_H
#define TETRIS_MENU_H
#include "menus.h"
#include "raylib.h"
#include "types.h"

extern GameState gameState;
extern Menu SettingsMenu;
extern Menu SoundMenu;
extern Menu StyleMenu;

// from main.c
void InitGameState(void);
void StartGame(void);
void QuitGame(void);
void ResumeFromPause(void);
void GoToMainMenu(void);

// from tetris_menus.c
void DrawMenu(Menu menu, Rectangle rec);
void LastMenu(MenuStack *ms);
void ShowScoreBoard(void);

#endif
