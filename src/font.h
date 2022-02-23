#include <SDL2/SDL.h>

struct font;

struct font* font_init(const char *path, int size, SDL_Renderer *renderer);
SDL_Point font_measure_text(const char *text, struct font *font);
SDL_Point font_measure_glyph(int glyph, struct font *font);
SDL_Point font_write_text(const char *text, SDL_Point xy, struct font *font, SDL_Renderer *renderer);
void font_deinit(struct font *font);
