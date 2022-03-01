#include "rect.h"

SDL_Rect rect_cut_left(SDL_Rect *r, int by)
{
    r->x += by;
    r->w -= by;

    return (SDL_Rect){r->x-by, r->y, by, r->h};
}

SDL_Rect rect_cut_right(SDL_Rect *r, int by)
{
    r->w -= by;

    return (SDL_Rect){r->x+r->w, r->y, by, r->h};
}

SDL_Rect rect_cut_top(SDL_Rect *r, int by)
{
    r->y += by;
    r->h -= by;

    return (SDL_Rect){r->x, r->y-by, r->h, by};
}

SDL_Rect rect_cut_bottom(SDL_Rect *r, int by)
{
    r->h -= by;

    return (SDL_Rect){r->x, r->y+r->h, r->w, by};
}

SDL_Rect rect_inset(SDL_Rect r, int by)
{
    return (SDL_Rect){r.x + by, r.y + by, r.w - by*2, r.h - by*2};
}

