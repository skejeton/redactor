#ifndef REDACTOR_DOCVIEW_H
#define REDACTOR_DOCVIEW_H
#include <SDL2/SDL_render.h>
#include "buffer.h"

struct docview {
    struct font *font;
    struct buffer buffer;
    float blink;
    // Tracking the change in position of a cursor to reset the blink
    struct buffer_cursor prev_cursor_pos;
    SDL_Point scroll;
};

void docview_draw(SDL_Rect viewport, SDL_Renderer *renderer, struct docview *view);
#endif
