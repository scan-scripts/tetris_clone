
#ifndef MENU_H
#define MENU_H
#include <stddef.h>
#define MENU_STACK_DEPTH 8

#define ARRAY_LEN(arr) ((int)sizeof(arr) / sizeof((arr)[0]))
#define ITEM_FUNC(label, fn) {.name = (label), .type = FUNC_ACTION, .action.func = (fn)}
#define ITEM_SUB(label, p_menu) {.name = (label), .type = MENU_ACTION, .action.subMenu = (p_menu)}
#define ITEM_SLIDER(label, p_slider) {.name = (label), .type = SLIDER_ACTION, .action.slider = (p_slider)}
#define ITEM_TOGGLE(label, p_toggle) {.name = (label), .type = TOGGLE_ACTION, .action.toggle = (p_toggle)}
#define ITEM_BACK(label) {.name = (label), .type = BACK_ACTION}

#define DEFINE_MENU(var, ttl, menu_items)                                                                              \
    Menu var = {                                                                                                       \
        .title = ttl, .headerText = NULL, .items = menu_items, .itemCount = ARRAY_LEN(menu_items), .selectedIndex = 0}

#define DEFINE_SLIDER(var, val, mn, mx, st, tg)                                                                        \
    MenuSlider var = (MenuSlider){.value = val, .min = mn, .max = mx, .step = st, .target = (int *)tg};

#define DEFINE_TOGGLE(var, itms, tg)                                                                                   \
    MenuToggle var = (MenuToggle){.items = itms, .count = ARRAY_LEN(itms), .selectedIndex = 0, .target = (int *)tg};

typedef enum ActionType_t { NO_ACTION, MENU_ACTION, FUNC_ACTION, SLIDER_ACTION, TOGGLE_ACTION, BACK_ACTION } ActionType;

typedef union MenuAction_t {
    void (*func)(void);
    struct Menu_t *subMenu;
    struct MenuSlider_t *slider;
    struct MenuToggle_t *toggle;
} MenuAction;

typedef struct MenuSlider_t {
    int value;
    int min;
    int max;
    int step;
    int *target;
} MenuSlider;

typedef struct ToggleItem_t {
    const char *label;
    int value;
} ToggleItem;

typedef struct MenuToggle_t {
    ToggleItem *items;
    int selectedIndex;
    int count;
    int *target;
} MenuToggle;

typedef struct MenuItem_t {
    const char *name;
    MenuAction action;
    ActionType type;
} MenuItem;

typedef struct Menu_t {
    const char *title;
    char *headerText;
    MenuItem *items;
    int itemCount;
    int selectedIndex;
} Menu;

typedef struct MenuStack_t {
    Menu *stack[MENU_STACK_DEPTH];
    int depth;
} MenuStack;

void MenuStackPush(MenuStack *ms, Menu *menu);
void MenuStackPop(MenuStack *ms);
Menu *MenuStackTop(MenuStack *ms);

void IncrementSlider(MenuSlider *slider);
void DecrementSlider(MenuSlider *slider);
void SetSlider(MenuSlider *slider, int value);

void ToggleCycleForward(MenuToggle *toggle);
void ToggleCycleBackward(MenuToggle *toggle);

#endif
