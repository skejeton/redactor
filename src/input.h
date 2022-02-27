#ifndef INPUT_H
#define INPUT_H
#include <stdbool.h>
#include "docview.h"
#include <SDL2/SDL.h>

struct input_pass {
    struct docview *view;
    SDL_Window *window;
    SDL_Event *event;
    const char *filename;
    bool *running;
};

struct input_state {
    bool ctrl, shift, leftmousedown, focused;
};

void input_process_event(struct input_state *state, struct input_pass pass);
#endif