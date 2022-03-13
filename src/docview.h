#ifndef REDACTOR_DOCVIEW_H
#define REDACTOR_DOCVIEW_H
#include <SDL2/SDL_render.h>
#include "docedit.h"
#include "ui.h"

struct docview_events {
    bool set_cursor_position_event_set;
    bool set_cursor_position_event_shift;
    bool set_cursor_position_event_reset_selection;
    SDL_Point set_cursor_position_point;
};

struct docview {
    // TODO: Make a scroll viewport
    SDL_Rect viewport;
    SDL_FPoint scroll;

    struct font *font;
    struct docedit document;

    struct docview_events events;
};

void dv_draw(struct docview *view, SDL_Renderer *renderer);
void dv_tap(struct docview *view, bool shift, SDL_Point xy);
void dv_scroll(struct docview *view, float dx, float dy);
#endif
