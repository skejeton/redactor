#ifndef REDACTOR_FONT_H
#define REDACTOR_FONT_H

#include <SDL2/SDL.h>

struct font;

int font_size(struct font *font);
struct font* font_resize(struct font *result, int size, SDL_Renderer *renderer);
struct font* font_init(const char *path, int size, SDL_Renderer *renderer);
SDL_Point font_measure_text(const char *text, struct font *font);
SDL_Point font_measure_glyph(int glyph, struct font *font);
SDL_Point font_write_text(const char *text, SDL_Point xy, SDL_Renderer *renderer, struct font *font);
void font_deinit(struct font *font);

#endif
