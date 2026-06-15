#include "scoreboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "types.h"

#define SCOREBOARD_FILE_NAME ".scoreboard.txt"
#define SCORE_LINE_BUFFER_LEN 24

extern GameState gameState;

static bool isDigit(char c) { return (c >= '0' && c <= '9'); }

static int ReadScoreLine(char *lineBuffer, ScoreElement *p_scoreEl) {
    char name[4];
    char scoreString[SCORE_LINE_BUFFER_LEN];
    int scoreIndex = 0;
    char delim = ',';
    bool reading_name = true;
    char c;
    for (int i = 0; i < SCORE_LINE_BUFFER_LEN; i++) {
        c = lineBuffer[i];
        if (reading_name) {
            if (c == delim) {
                reading_name = false;
                name[i] = '\0';
                continue;
            }
            if (i >= 3) {
                return -1;
            }
            name[i] = c;
        } else {
            if (isDigit(c)) {
                if (scoreIndex >= SCORE_LINE_BUFFER_LEN - 1) {
                    return -1;
                }
                scoreString[scoreIndex] = c;
                scoreIndex++;
                continue;
            }
            if (c == '\n' || c == '\0') {
                if (scoreIndex == 0) {
                    return -1;
                }
                scoreString[scoreIndex] = '\0';
                strncpy(p_scoreEl->name, name, sizeof(name));
                p_scoreEl->score = (int)strtol(scoreString, NULL, 10);
                return 0;
            } else {
                return -1;
            }
        }
    }
    return -1;
}

void WriteScoreBoardToFile(void) {
    FILE *p_file = fopen(SCOREBOARD_FILE_NAME, "w");
    if (p_file == NULL) {
        return;
    }
    for (int i = 0; i < N_SCORE_ELS; i++) {
        fprintf(p_file, "%s,%d\n", gameState.scoreBoard.scores[i].name, gameState.scoreBoard.scores[i].score);
    }
    fclose(p_file);
}

int ReadScoreBoardFile(void) {
    FILE *p_file = fopen(SCOREBOARD_FILE_NAME, "r");
    if (p_file == NULL) {
        return -1;
    }
    char lineBuffer[SCORE_LINE_BUFFER_LEN];
    for (int i = 0; i < N_SCORE_ELS; i++) {
        if ((fgets(lineBuffer, sizeof(lineBuffer), p_file) == NULL)) {
            fclose(p_file);
            return -1 * (i + 1);
        }
        if (ReadScoreLine(lineBuffer, &(gameState.scoreBoard.scores[i]))) {
            fclose(p_file);
            return -1;
        }
    }
    fclose(p_file);
    return 0;
}

int CheckHighScore(int score) {
    for (int i = 0; i < N_SCORE_ELS; i++) {
        if (score > gameState.scoreBoard.scores[i].score) {
            return i;
        }
    }
    return -1;
}

void ScoreBoardToString(char *s) {
    s[0] = '\0';
    for (int i = 0; i < N_SCORE_ELS; i++) {
        sprintf(s + strlen(s), "%s: %d\n", gameState.scoreBoard.scores[i].name, gameState.scoreBoard.scores[i].score);
    }
}

void addHighScore(int place, char *name, int score) {
    for (int i = N_SCORE_ELS - 1; i > place; i--) {
        gameState.scoreBoard.scores[i] = gameState.scoreBoard.scores[i - 1];
    }
    strncpy(gameState.scoreBoard.scores[place].name, name, 3);
    gameState.scoreBoard.scores[place].name[3] = '\0';
    gameState.scoreBoard.scores[place].score = score;
}

void InitScoreBoard(void) {
    gameState.scoreBoard = (ScoreBoard){{{"AAA", 100}, {"BBB", 20}, {"CCC", 10}, {"DDD", 3}, {"EEE", 2}, {"FFF", 1}}};
    if (ReadScoreBoardFile() != 0) {
        WriteScoreBoardToFile();
    }
}

static bool GetUserText(char *nameBuffer, int n) {
    static int letterCount = 0;
    int key = GetCharPressed();

    while (key > 0) {
        if ((key >= 32) && (key <= 125) && (letterCount < n) && (char)key != ' ') {
            nameBuffer[letterCount] = (char)key;
            nameBuffer[letterCount + 1] = '\0';
            letterCount++;
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        letterCount--;
        if (letterCount < 0)
            letterCount = 0;
        nameBuffer[letterCount] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER)) {
        printf("%s\n", nameBuffer);
        letterCount = 0;
        return true;
    }
    return false;
}

bool ShowHighScoreMenu(int score_place) {
    static char nameBuffer[4] = {0};
    Rectangle rec = {0, 100, 800, 600};
    bool nameEntered = GetUserText(nameBuffer, 3);
    DrawRectangleRec(rec, GREEN);
    DrawText(TextFormat("NEW HIGH SCORE \nENTER YOUR INITALs \n   %s", nameBuffer), 300, 300, 30, BROWN);
    if (nameEntered) {
        addHighScore(score_place, nameBuffer, gameState.score);
        WriteScoreBoardToFile();
        gameState.scoreAdded = true;
        return true;
    }
    return false;
}
