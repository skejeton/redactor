#include <SDL2/SDL_rect.h>

SDL_Rect rect_cut_left(SDL_Rect *r, int by);
SDL_Rect rect_cut_right(SDL_Rect *r, int by);
SDL_Rect rect_cut_top(SDL_Rect *r, int by);
SDL_Rect rect_cut_bottom(SDL_Rect *r, int by);
SDL_Rect rect_inset(SDL_Rect rect, int by);
SDL_Rect rect_center(SDL_Rect parent, SDL_Rect child);
SDL_Point rect_position(SDL_Rect r);
SDL_Point rect_size(SDL_Rect r);
SDL_Point point_add(SDL_Point a, SDL_Point b);
SDL_Point point_sub(SDL_Point a, SDL_Point b);
SDL_Rect rect_offset(SDL_Rect r, SDL_Point p);
SDL_Rect rect_keep_in_bounds(SDL_Rect parent, SDL_Rect child);

