
#ifndef TETRIS_MENU_H
#define TETRIS_MENU_H
#include "menus.h"
#include "raylib.h"
#include "types.h"

extern GameState gameState;
extern Menu settingsMenu;
extern Menu soundMenu;
extern Menu styleMenu;

// from main.c
void InitGameState(void);
void StartGame(void);
void QuitGame(void);
void ResumeFromPause(void);
void GoToMainMenu(void);

// from tetris_menus.c
void DrawMenu(Menu menu, Rectangle rec);
void ShowScoreBoard(void);
void UpdateMenu();
extern Menu mainMenu;
extern Menu pauseMenu;
extern Menu scoreboardMenu;
extern Menu gameOverMenu;
void PushScoreboardMenu(void);
void PushGameOverMenu(void);
void PushNameEntryMenu(int place);
#endif
