#include "tetris_menus.h"
// #include "menus.h"
#include "raylib.h"

extern GameState gameState;
extern ScoreBoard scoreBoard;

void UpdateMenu(MenuStack *ms) {
    Menu *currentMenu = MenuStackTop(ms);
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
            MenuStackPush(ms, currentItem->action.subMenu);
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
        MenuStackPop(ms);
        break;
    default:
        break;
    }
}

void DrawMenuToggle(MenuToggle toggle, Rectangle rec) {
    // if there is 0 or 1 items in the menu we don't draw the box
    return;
}
void DrawMenuSlider(MenuSlider slider, Rectangle rec) { return; }
void DrawMenuItem(MenuItem menuItem, Rectangle rec) { return; }

void DrawMenu(Menu menu, Rectangle rec) { return; }
void ShowScoreBoard(void) {}

MenuItem mainItems[] = {
    ITEM_FUNC("Start", StartGame),
    ITEM_SUB("Settings", &SettingsMenu),
    ITEM_FUNC("ScoreBoard", ShowScoreBoard),
    ITEM_FUNC("Quit", QuitGame),
};

DEFINE_MENU(MainMenu, "TETRIS", mainItems);

MenuItem settingsItems[] = {ITEM_SUB("Sound", &SoundMenu), ITEM_SUB("Style", &StyleMenu), ITEM_FUNC("Back", LastMenu)};
DEFINE_MENU(settingsMenu, "SETTINGS", settingsItems);

DEFINE_SLIDER(musicSlider, 50, 0, 100, 1, &gameState.musicVolume);
DEFINE_SLIDER(sfxSlider, 50, 0, 100, 1, &gameState.sfxVolume);

ToggleItem soundStyleItems[] = {{"Classic", CLASSIC}, {"Silly", SILLY}};
DEFINE_TOGGLE(musicStyleToggle, soundStyleItems, &gameState.musicStyle);
DEFINE_TOGGLE(sfxStyleToggle, soundStyleItems, &gameState.sfxStyle);

MenuItem soundItems[] = {
    ITEM_SLIDER("Music Volume", &musicSlider),
    ITEM_TOGGLE("Music Style", &musicStyleToggle),
    ITEM_SLIDER("SFX Volume", &sfxSlider),
    ITEM_TOGGLE("SFX Style", &sfxStyleToggle),
    ITEM_FUNC("Back", LastMenu),
};
DEFINE_MENU(SoundMenu, "SOUND", soundItems);
Menu StyleMenu = {0};

MenuItem pauseMenuItems[] = {ITEM_FUNC("Resume", ResumeFromPause), ITEM_FUNC("Restart", InitGameState),
                             ITEM_SUB("Settings", &settingsMenu), ITEM_FUNC("Main Menu", GoToMainMenu)};

DEFINE_MENU(pauseMenu, "PAUSE", pauseMenuItems);
