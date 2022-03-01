#include "ui.h"
#include "rect.h"

struct view ui_default_view(int x, int y, int w, int h) 
{
    return (struct view) {{ x, y, w, h }};
}

struct view ui_default_view_rec(SDL_Rect rect) 
{
    return (struct view) {rect};
}

struct view ui_cut_right(struct view *view, int by)
{
    return ui_default_view_rec(rect_cut_right(&view->rect_, by));
}

struct view ui_cut_top(struct view *view, int by)
{
    return ui_default_view_rec(rect_cut_top(&view->rect_, by));
}

struct view ui_cut_bottom(struct view *view, int by)
{
    return ui_default_view_rec(rect_cut_bottom(&view->rect_, by));
}

struct view ui_cut_left(struct view *view, int by)
{
    return ui_default_view_rec(rect_cut_left(&view->rect_, by));
}

void ui_inset(struct view *view, int by)
{
    view->rect_ = rect_inset(view->rect_, by);
}

struct SDL_Rect ui_get_view_rect(struct view *view)
{
    return rect_inset(view->rect_, 2);
}

struct SDL_Rect ui_center_y(struct view *view, SDL_Rect r)
{
    r.y += (view->rect_.h - r.h) / 2;
    return r;
}
