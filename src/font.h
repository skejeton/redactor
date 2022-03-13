#ifndef REDACTOR_FONT_H
#define REDACTOR_FONT_H

#include <SDL2/SDL.h>

struct font;

struct font* font_init(const char *path, int size, SDL_Renderer *renderer);
void font_deinit(struct font *font);
struct font* font_resize(struct font *result, int size, SDL_Renderer *renderer);

int font_get_size(struct font *font);
int font_get_height(struct font *font);
SDL_Point font_measure_text(struct font *font, const char *text);
SDL_Point font_measure_glyph(struct font *font, int glyph);
SDL_Point font_write_text(struct font *font, const char *text, SDL_Point xy, SDL_Renderer *renderer);

#endif
