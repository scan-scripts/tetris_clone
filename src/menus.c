#include "menus.h"
// #include "types.h"
#include <assert.h>
// #include <stddef.h>

void ValidateMenuToggle(MenuToggle *p_toggle) {
    assert(p_toggle->items != NULL);
    assert(p_toggle->count > 0);
    assert(p_toggle->target != NULL);
}

void ValidateMenuSlider(MenuSlider *p_slider) {
    assert(p_slider->min < p_slider->max);
    assert(p_slider->value <= p_slider->max);
    // assert(p_slider->value >= p_slider->min);
    assert(p_slider->step > 0);
    assert(p_slider->step <= (p_slider->max - p_slider->min));
    assert(((p_slider->max - p_slider->min) % p_slider->step) == 0);
    assert(((p_slider->value - p_slider->min) % p_slider->step) == 0);
}

void MenuStackPush(MenuStack *ms, Menu *menu) {
    assert(ms->depth < MENU_STACK_DEPTH);
    ms->stack[++ms->depth] = menu;
}

void MenuStackPop(MenuStack *ms) {
    if (ms->depth >= 0) {
        ms->depth--;
    }
}

Menu *MenuStackTop(MenuStack *ms) {
    if (ms->depth < 0) {
        return NULL;
    }
    return ms->stack[ms->depth];
}
void IncrementSlider(MenuSlider *slider) {
    int val = slider->value + slider->step;
    if (val <= slider->max) {
        slider->value = val;
        if (slider->target != NULL) {
            *slider->target = slider->value;
        }
    }
}

void DecrementSlider(MenuSlider *slider) {
    int val = slider->value;
    val -= slider->step;
    if (val >= slider->min) {
        slider->value = val;
        if (slider->target != NULL) {
            *slider->target = slider->value;
        }
    }
}

void SetSlider(MenuSlider *slider, int value) {
    assert((value >= slider->min) && (value <= slider->max));
    // value = ((value + (slider->step / 2)) / slider->step) * slider->step; //maybe remove this
    slider->value = value;
    if (slider->target != NULL) {
        *slider->target = slider->value;
    }
}

void ToggleCycleForward(MenuToggle *toggle) {
    toggle->selectedIndex = (toggle->selectedIndex + 1) % toggle->count;
    if (toggle->target != NULL) {
        *toggle->target = (toggle->items[toggle->selectedIndex].value);
    }
}

void ToggleCycleBackward(MenuToggle *toggle) {
    toggle->selectedIndex = (toggle->selectedIndex - 1 + toggle->count) % toggle->count;
    if (toggle->target != NULL) {
        *toggle->target = (toggle->items[toggle->selectedIndex].value);
    }
}
