#ifndef REDACTOR_DOCVIEW_H
#define REDACTOR_DOCVIEW_H
#include <SDL2/SDL_render.h>
#include "docedit.h"

struct docview {
    struct font *font;
    struct docedit doc;
    float blink;
    // Tracking the change in position of a cursor to reset the blink
    struct buffer_marker prev_cursor_pos;
    SDL_FPoint scroll;
    SDL_FPoint scroll_damped;
};

void docview_draw(SDL_Rect viewport, SDL_Renderer *renderer, struct docview *view);
void docview_tap(bool shift, SDL_Rect viewport, SDL_Point xy, struct docview *view);
#endif
