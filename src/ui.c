#include "ui.h"

struct SDL_Rect inner_rect(SDL_Rect r, int by) 
{
    return (SDL_Rect) { r.x + by, r.y + by, r.w - by*2, r.h - by*2 };
}

struct view ui_default_view(int x, int y, int w, int h) 
{
    return (struct view) {{ x, y, w, h }};
}

struct view ui_cut_bottom(struct view *view, int by)
{
    view->rect_.h -= by;

    SDL_Rect rect = view->rect_;

    return ui_default_view(rect.x, rect.y + rect.h, rect.w, by);
}

void ui_inset(struct view *view, int by)
{
    view->rect_ = inner_rect(view->rect_, by);
}

struct SDL_Rect ui_get_view_rect(struct view *view)
{
    return inner_rect(view->rect_, 2);
}

struct SDL_Rect ui_center_y(struct view *view, SDL_Rect r)
{
    r.y += (view->rect_.h - r.h) / 2;
    return r;
}
