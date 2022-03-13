#ifndef REDACTOR_DOCVIEW_H
#define REDACTOR_DOCVIEW_H
#include <SDL2/SDL_render.h>
#include "docedit.h"
#include "ui.h"

struct docview {
    // TODO: Make a scroll viewport
    SDL_Rect viewport;
    SDL_FPoint scroll;

    struct font *font;
    struct docedit document;
};

void dv_draw(struct docview *view, SDL_Renderer *renderer);
void dv_tap(struct docview *view, bool shift, SDL_Point xy);
void dv_scroll(struct docview *view, float dx, float dy);
#endif
