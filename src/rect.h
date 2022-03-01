#include <SDL2/SDL_rect.h>

SDL_Rect rect_cut_left(SDL_Rect *r, int by);
SDL_Rect rect_cut_right(SDL_Rect *r, int by);
SDL_Rect rect_cut_top(SDL_Rect *r, int by);
SDL_Rect rect_cut_bottom(SDL_Rect *r, int by);
SDL_Rect rect_inset(SDL_Rect rect, int by);
