#ifndef REDACTOR_DOCVIEW_H
#define REDACTOR_DOCVIEW_H
#include <SDL2/SDL_render.h>
#include "docedit.h"
#include "ui.h"

struct docview {
    SDL_Rect viewport;
    SDL_FPoint viewport_scroll;
    struct font *font;
    struct docedit document;
    int number_line_width;
};

void dv_draw(struct docview *view, SDL_Renderer *renderer);
void dv_tap(struct docview *view, bool shift, SDL_Point xy);
void dv_scroll(struct docview *view, float dx, float dy);
#endif
