#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "types.h"
#include <stdbool.h>

void InitScoreBoard(void);
int ReadScoreBoardFile(void);
void WriteScoreBoardToFile(void);
int CheckHighScore(int score);
void ScoreBoardToString(char *s);
void addHighScore(int place, char *name, int score);
bool ShowHighScoreMenu(int score_place);

#endif
