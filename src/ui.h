#include <SDL2/SDL_rect.h>

struct view {
    SDL_Rect rect_;
};

struct view ui_default_view(int x, int y, int w, int h) ;
struct view ui_default_view_rec(SDL_Rect rect);
struct view ui_cut_right(struct view *view, int by);
struct view ui_cut_top(struct view *view, int by);
struct view ui_cut_bottom(struct view *view, int by);
struct view ui_cut_left(struct view *view, int by);
void ui_inset(struct view *view, int by);

// Will handle out the paddings for ya
struct SDL_Rect ui_get_view_rect(struct view *view);
struct SDL_Rect ui_center_y(struct view *view, SDL_Rect r);
