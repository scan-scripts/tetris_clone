#include "tetris_menus.h"
#include "raylib.h"
#include "scoreboard.h"
#include <stdio.h>

extern GameState gameState;

char scoreBoardBuffer[64 * 8];

void UpdateMenu() {
    static float heldTimer = 0.0f;
    static int heldDir = 0;
    float dT = GetFrameTime();
    float holdDelay = 0.3f;
    float holdInterval = 0.05f;

    Menu *currentMenu = MenuStackTop(&gameState.menuStack);
    if (!currentMenu)
        return;
    if (IsKeyPressed(KEY_UP)) {
        currentMenu->selectedIndex =
            ((currentMenu->selectedIndex - 1) + currentMenu->itemCount) % +currentMenu->itemCount;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        currentMenu->selectedIndex = ((currentMenu->selectedIndex + 1)) % +currentMenu->itemCount;
    }
    MenuItem *currentItem = &currentMenu->items[currentMenu->selectedIndex];

    int dir = 0;
    if (IsKeyDown(KEY_RIGHT))
        dir = 1;
    if (IsKeyDown(KEY_LEFT))
        dir = -1;

    bool fireRepeat = false;
    if (dir != 0) {
        if (dir != heldDir) {
            heldTimer = 0.0f;
            heldDir = dir;
        }
        heldTimer += dT;
        if (heldTimer >= holdDelay) {
            fireRepeat = true;
            heldTimer = holdDelay - holdInterval;
        }
    } else {
        heldDir = 0;
        heldTimer = 0.0f;
    }

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
        if (IsKeyPressed(KEY_RIGHT) || (fireRepeat && dir == 1)) {
            IncrementSlider(currentItem->action.slider);
        }
        if (IsKeyPressed(KEY_LEFT) || (fireRepeat && dir == -1)) {
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

// Doesn't work with new lines!
void DrawTextCenterAligned(const char *text, int fontSize, Rectangle rec, float *p_yOffset, Color color) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, fontSize / 10.0f);
    int textX = rec.x + (rec.width / 2.0) - (textSize.x / 2.0);
    DrawText(text, textX, rec.y + (*p_yOffset), fontSize, color);
    *p_yOffset += textSize.y;
}

// Doesn't work with new lines!
void DrawTextAtCursor(const char *text, Vector2 *p_cursor, int fontSize, Color color) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, fontSize / 10.0f);
    DrawText(text, p_cursor->x, p_cursor->y, fontSize, color);
    p_cursor->x += textSize.x;
    // p_cursor->y += textSize.y;
}

void DrawMenuSlider(MenuItem menuItem, int fontSize, Rectangle rec, float *p_yOffset, Color color) {
    MenuSlider *s = menuItem.action.slider;
    float spacing = fontSize / 10.0f;

    const char *valueStr = TextFormat("%s: [< %d >]", menuItem.name, s->value);
    float labelWidth = MeasureTextEx(GetFontDefault(), valueStr, fontSize, spacing).x;
    float labelX = rec.x + (rec.width / 2.0f) - (labelWidth / 2.0f);
    DrawText(valueStr, labelX, rec.y + *p_yOffset, fontSize, color);
    *p_yOffset += fontSize + spacing;

    float barWidth = 200.0f;
    float barHeight = fontSize * 0.6f;
    float barX = rec.x + (rec.width / 2.0f) - (barWidth / 2.0f);
    float fillWidth = barWidth * (float)(s->value - s->min) / (float)(s->max - s->min);

    DrawRectangle(barX, rec.y + *p_yOffset, barWidth, barHeight, Fade(color, 0.25f));
    DrawRectangle(barX, rec.y + *p_yOffset, fillWidth, barHeight, color);
    *p_yOffset += barHeight;
}

void DrawMenuToggle(MenuItem menuItem, int fontSize, Rectangle rec, float *p_yOffset, Color color) {
    float spacing = fontSize / 10.0f;
    float totalTextWidth = 0;
    totalTextWidth += MeasureTextEx(GetFontDefault(), TextFormat("%s:", menuItem.name), fontSize, spacing).x;
    for (int i = 0; i < menuItem.action.toggle->count; i++) {
        totalTextWidth += MeasureTextEx(GetFontDefault(), " ", fontSize, spacing).x;
        totalTextWidth += MeasureTextEx(GetFontDefault(), "[", fontSize, spacing).x;
        totalTextWidth += MeasureTextEx(GetFontDefault(), menuItem.action.toggle->items[i].label, fontSize, spacing).x;
        totalTextWidth += MeasureTextEx(GetFontDefault(), "]", fontSize, spacing).x;
    }

    Vector2 cur = {rec.x + (rec.width / 2.0) - (totalTextWidth / 2.0), rec.y + *p_yOffset};
    DrawTextAtCursor(TextFormat("%s:", menuItem.name), &cur, fontSize, color);
    Color braketColor;
    for (int i = 0; i < menuItem.action.toggle->count; i++) {
        if (i == menuItem.action.toggle->selectedIndex) {
            braketColor = color;
        } else {
            braketColor = (Color){0, 0, 0, 0};
        }
        DrawTextAtCursor(" ", &cur, fontSize, color);
        DrawTextAtCursor("[", &cur, fontSize, braketColor);
        DrawTextAtCursor(menuItem.action.toggle->items[i].label, &cur, fontSize, color);
        DrawTextAtCursor("]", &cur, fontSize, braketColor);
    }
    *p_yOffset += fontSize;
}

void DrawMenuItem(MenuItem menuItem, int fontSize, Rectangle rec, float *p_yOffset, Color color) {
    if (menuItem.type == TEXT_INPUT_ACTION) {
        char displayStr[64];
        snprintf(displayStr, sizeof(displayStr), "%s: [%s]", menuItem.name, menuItem.action.textInput->buffer);
        DrawTextCenterAligned(displayStr, fontSize, rec, p_yOffset, color);
    } else if (menuItem.type == SLIDER_ACTION) {
        DrawMenuSlider(menuItem, fontSize, rec, p_yOffset, color);
    } else if (menuItem.type == TOGGLE_ACTION) {
        // DrawTextCenterAligned(menuItem.name, fontSize, rec, p_yOffset, color);
        DrawMenuToggle(menuItem, fontSize, rec, p_yOffset, color);

    } else {
        DrawTextCenterAligned(menuItem.name, fontSize, rec, p_yOffset, color);
    }
}

// mouse controls for menus:
// - hovering should highlight the menu

void DrawMenu(Menu menu, Rectangle rec) {
    int titleFontSize = 60;
    float titleSpacing = 30.0f;
    int itemFontSize = 20;
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
    snprintf(gameOverBuffer, sizeof(gameOverBuffer), "Score: %d\n\n%s", gameState.score, scoreboardStr);
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

DEFINE_SLIDER(musicSlider, 50, 0, 100, 1, &gameState.settings.musicVolume);
DEFINE_SLIDER(sfxSlider, 50, 0, 100, 1, &gameState.settings.sfxVolume);

ToggleItem soundStyleItems[] = {{"Classic", CLASSIC}, {"Silly", SILLY}, {"None", NONE}};
DEFINE_TOGGLE(musicStyleToggle, soundStyleItems, &gameState.settings.musicStyle);
DEFINE_TOGGLE(sfxStyleToggle, soundStyleItems, &gameState.settings.sfxStyle);

MenuItem soundItems[] = {ITEM_SLIDER("Music Volume", &musicSlider), ITEM_TOGGLE("Music Style", &musicStyleToggle),
                         ITEM_SLIDER("SFX Volume", &sfxSlider), ITEM_TOGGLE("SFX Style", &sfxStyleToggle),
                         ITEM_BACK("Back")};

DEFINE_MENU(soundMenu, "SOUND SETTINGS", soundItems);

ToggleItem onOffToggleItems[] = {{"On", true}, {"Off", false}};

DEFINE_TOGGLE(showShadowToggle, onOffToggleItems, &gameState.settings.showShadow);
DEFINE_TOGGLE(allowHoldToggle, onOffToggleItems, &gameState.settings.allowHold);

DEFINE_SLIDER(nextCountSlider, 1, 0, 3, 1, &gameState.settings.nextCount);

ToggleItem randomTypeToggleItems[] = {{"Random", false}, {"Shuffle", true}};

DEFINE_TOGGLE(randomTypeToggle, randomTypeToggleItems, &gameState.settings.randomShuffle);

MenuItem gameModeItems[] = {ITEM_TOGGLE("Show Shadow", &showShadowToggle),
                            ITEM_SLIDER("Show Next Count", &nextCountSlider),
                            ITEM_TOGGLE("Allow Hold", &allowHoldToggle), ITEM_TOGGLE("Random Type", &randomTypeToggle),

                            ITEM_BACK("Back")};

DEFINE_MENU(styleMenu, "GAME MODE SETTINGS", gameModeItems);

MenuItem pauseMenuItems[] = {ITEM_FUNC("Resume", ResumeFromPause), ITEM_FUNC("Restart", InitGameState),
                             ITEM_SUB("Settings", &settingsMenu), ITEM_FUNC("Main Menu", GoToMainMenu)};

DEFINE_MENU(pauseMenu, "PAUSE", pauseMenuItems);
