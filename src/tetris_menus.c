#include "tetris_menus.h"
#include "scoreboard.h"
#include "raylib.h"
#include <stdio.h>

extern GameState gameState;

char scoreBoardBuffer[64 * 8];

void UpdateMenu() {
    Menu *currentMenu = MenuStackTop(&gameState.menuStack);
    if (!currentMenu) return;
    if (IsKeyPressed(KEY_UP)) {
        currentMenu->selectedIndex =
            ((currentMenu->selectedIndex - 1) + currentMenu->itemCount) % +currentMenu->itemCount;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        currentMenu->selectedIndex = ((currentMenu->selectedIndex + 1)) % +currentMenu->itemCount;
    }
    MenuItem *currentItem = &currentMenu->items[currentMenu->selectedIndex];
    switch (currentItem->type) {
    case NO_ACTION:
        break;
    case MENU_ACTION:
        if (IsKeyPressed(KEY_ENTER)) {
            MenuStackPush(&gameState.menuStack, currentItem->action.subMenu);
        }
        break;
    case FUNC_ACTION:
        if (IsKeyPressed(KEY_ENTER)) {
            currentItem->action.func();
        }

        break;
    case SLIDER_ACTION:
        if (IsKeyPressed(KEY_RIGHT)) {
            IncrementSlider(currentItem->action.slider);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            DecrementSlider(currentItem->action.slider);
        }
        break;
    case TOGGLE_ACTION:
        if (IsKeyPressed(KEY_RIGHT)) {
            ToggleCycleForward(currentItem->action.toggle);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            ToggleCycleBackward(currentItem->action.toggle);
        }
        break;
    case BACK_ACTION:
        if (IsKeyPressed(KEY_ENTER)) {
            MenuStackPop(&gameState.menuStack);
        }
        break;
    case TEXT_INPUT_ACTION: {
        MenuTextInput *input = currentItem->action.textInput;
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 125 && key != ' ' && input->len < input->maxLen) {
                input->buffer[input->len++] = (char)key;
                input->buffer[input->len] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && input->len > 0) {
            input->buffer[--input->len] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER) && input->len > 0 && input->onConfirm) {
            input->onConfirm();
        }
        break;
    }
    default:
        break;
    }
}

MenuItem scoreboardItems[] = {ITEM_BACK("Back")};
DEFINE_MENU_W_HEADER(scoreboardMenu, "HIGH SCORES", scoreBoardBuffer, scoreboardItems);

void PushScoreboardMenu(void) {
    ScoreBoardToString(scoreBoardBuffer);
    MenuStackPush(&gameState.menuStack, &scoreboardMenu);
}

// Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, fontSize / 10.0f);
//
// int textX = rec.x + (rec.width / 2.0) - (textSize.x / 2.0);
// int textY = rec.y + (rec.height / 2.0) - (textSize.y / 2.0);
// DrawText(text, textX, textY, fontSize, color);

void DrawTextCenterAligned(const char *text, int fontSize, Rectangle rec, float *p_yOffset, Color color) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, fontSize / 10.0f);
    int textX = rec.x + (rec.width / 2.0) - (textSize.x / 2.0);
    DrawText(text, textX, rec.y + (*p_yOffset), fontSize, color);
    *p_yOffset += textSize.y;
}

void DrawMenuToggle(MenuToggle toggle, int fontSize, Rectangle rec, float *p_yOffset, Color color) { return; }

void DrawMenuSlider(MenuSlider slider, int fontSize, Rectangle rec, float *p_yOffset, Color color) { return; }

void DrawMenuItem(MenuItem menuItem, int fontSize, Rectangle rec, float *p_yOffset, Color color) {
    if (menuItem.type == TEXT_INPUT_ACTION) {
        char displayStr[64];
        snprintf(displayStr, sizeof(displayStr), "%s: [%s]", menuItem.name, menuItem.action.textInput->buffer);
        DrawTextCenterAligned(displayStr, fontSize, rec, p_yOffset, color);
    } else if (menuItem.type == SLIDER_ACTION) {
        DrawTextCenterAligned(menuItem.name, fontSize, rec, p_yOffset, color);
    } else if (menuItem.type == TOGGLE_ACTION) {
        DrawTextCenterAligned(menuItem.name, fontSize, rec, p_yOffset, color);
    } else {
        DrawTextCenterAligned(menuItem.name, fontSize, rec, p_yOffset, color);
    }
}

void DrawMenu(Menu menu, Rectangle rec) {
    int titleFontSize = 60;
    float titleSpacing = 30.0f;
    int itemFontSize = 40;
    float itemSpacing = 20.0f;
    Color baseColor = WHITE;
    Color highlightColor = GREEN;
    float spacing = (float)itemFontSize / 10.0f;

    // compute total height to vertically center the content
    float totalHeight = 0;
    if (!(menu.title == NULL || menu.title[0] == '\0')) {
        totalHeight += MeasureTextEx(GetFontDefault(), menu.title, titleFontSize, titleFontSize / 10.0f).y;
        totalHeight += titleSpacing;
    }
    if (!(menu.headerText == NULL || menu.headerText[0] == '\0')) {
        totalHeight += MeasureTextEx(GetFontDefault(), menu.headerText, itemFontSize, spacing).y;
        totalHeight += titleSpacing;
    }
    for (int i = 0; i < menu.itemCount; i++) {
        totalHeight += MeasureTextEx(GetFontDefault(), menu.items[i].name, itemFontSize, spacing).y;
        totalHeight += itemSpacing;
    }

    float yOffset = (rec.height - totalHeight) / 2.0f;

    if (!(menu.title == NULL || menu.title[0] == '\0')) {
        DrawTextCenterAligned(menu.title, titleFontSize, rec, &yOffset, baseColor);
        yOffset += titleSpacing;
    }
    if (!(menu.headerText == NULL || menu.headerText[0] == '\0')) {
        DrawTextCenterAligned(menu.headerText, itemFontSize, rec, &yOffset, baseColor);
        yOffset += titleSpacing;
    }
    for (int i = 0; i < menu.itemCount; i++) {
        DrawMenuItem(menu.items[i], itemFontSize, rec, &yOffset, menu.selectedIndex == i ? highlightColor : baseColor);
        yOffset += itemSpacing;
    }
}

char gameOverBuffer[64 * 8 + 32];

MenuItem gameOverItems[] = {
    ITEM_FUNC("Play Again", StartGame),
    ITEM_FUNC("Main Menu", GoToMainMenu),
};
DEFINE_MENU_W_HEADER(gameOverMenu, "GAME OVER", gameOverBuffer, gameOverItems);

void PushGameOverMenu(void) {
    char scoreboardStr[64 * 8];
    ScoreBoardToString(scoreboardStr);
    snprintf(gameOverBuffer, sizeof(gameOverBuffer), "Score: %d\n%s", gameState.score, scoreboardStr);
    gameOverMenu.selectedIndex = 0;
    MenuStackPush(&gameState.menuStack, &gameOverMenu);
}

static char nameBuffer[4];
static int nameEntryScorePlace;
static MenuTextInput nameInput;

void OnNameConfirmed(void) {
    addHighScore(nameEntryScorePlace, nameBuffer, gameState.score);
    WriteScoreBoardToFile();
    MenuStackPop(&gameState.menuStack);
    PushGameOverMenu();
}

MenuItem nameEntryItems[] = {ITEM_TEXT_INPUT("Initials", &nameInput)};
DEFINE_MENU(nameEntryMenu, "NEW HIGH SCORE", nameEntryItems);

void PushNameEntryMenu(int place) {
    nameEntryScorePlace = place;
    nameBuffer[0] = '\0';
    nameInput = (MenuTextInput){.buffer = nameBuffer, .maxLen = 3, .len = 0, .onConfirm = OnNameConfirmed};
    nameEntryMenu.selectedIndex = 0;
    MenuStackPush(&gameState.menuStack, &nameEntryMenu);
}

MenuItem mainItems[] = {
    ITEM_FUNC("Start", StartGame),
    ITEM_SUB("Settings", &settingsMenu),
    ITEM_FUNC("ScoreBoard", PushScoreboardMenu),
    ITEM_FUNC("Quit", QuitGame),
};

DEFINE_MENU(mainMenu, "TETRIS", mainItems);

MenuItem settingsItems[] = {ITEM_SUB("Sound", &soundMenu), ITEM_SUB("Style", &styleMenu), ITEM_BACK("Back")};
DEFINE_MENU(settingsMenu, "SETTINGS", settingsItems);

DEFINE_SLIDER(musicSlider, 50, 0, 100, 1, &gameState.musicVolume);
DEFINE_SLIDER(sfxSlider, 50, 0, 100, 1, &gameState.sfxVolume);

ToggleItem soundStyleItems[] = {{"Classic", CLASSIC}, {"Silly", SILLY}};
DEFINE_TOGGLE(musicStyleToggle, soundStyleItems, &gameState.musicStyle);
DEFINE_TOGGLE(sfxStyleToggle, soundStyleItems, &gameState.sfxStyle);

MenuItem soundItems[] = {ITEM_SLIDER("Music Volume", &musicSlider), ITEM_TOGGLE("Music Style", &musicStyleToggle),
                         ITEM_SLIDER("SFX Volume", &sfxSlider), ITEM_TOGGLE("SFX Style", &sfxStyleToggle),
                         ITEM_BACK("Back")};

DEFINE_MENU(soundMenu, "SOUND", soundItems);
Menu styleMenu = {0};

MenuItem pauseMenuItems[] = {ITEM_FUNC("Resume", ResumeFromPause), ITEM_FUNC("Restart", InitGameState),
                             ITEM_SUB("Settings", &settingsMenu), ITEM_FUNC("Main Menu", GoToMainMenu)};

DEFINE_MENU(pauseMenu, "PAUSE", pauseMenuItems);
