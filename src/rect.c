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

SDL_Rect rect_center(SDL_Rect parent, SDL_Rect child)
{
    child.x = parent.x + (parent.w-child.w)/2;
    child.y = parent.y + (parent.h-child.h)/2;
    return child;
}

SDL_Point rect_position(SDL_Rect r)
{
    return (SDL_Point){r.x, r.y};
}

SDL_Point rect_size(SDL_Rect r)
{
    return (SDL_Point){r.w, r.h};
}

SDL_Point point_add(SDL_Point a, SDL_Point b)
{
    return (SDL_Point){a.x+b.x, a.y+b.y};
}

SDL_Point point_sub(SDL_Point a, SDL_Point b)
{
    return (SDL_Point){a.x-b.x, a.y-b.y};
}

SDL_Rect rect_offset(SDL_Rect r, SDL_Point p)
{
    return (SDL_Rect){r.x+p.x, r.y+p.y, r.w, r.h};
}

SDL_Rect rect_keep_in_bounds(SDL_Rect parent, SDL_Rect child)
{
    if (child.x < parent.x)
        child.x = parent.x;
    if (child.y < parent.y)
        child.y = parent.y;
    if (child.x + child.w > parent.x + parent.w)
        child.x = parent.x + parent.w - child.w;
    if (child.y + child.h > parent.y + parent.h)
        child.y = parent.y + parent.h - child.h;
    return child;
}
