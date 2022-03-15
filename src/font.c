#include "font.h"
#include "glyphmap.h"
#include "utf8.h"

struct font {
    struct glyphmap map;
    char *font_path;
    int font_size;
};

struct font* font_init(const char *path, int size, SDL_Renderer *renderer)
{
    struct font *result = calloc(sizeof(struct font), 1);
    result->font_path = strcpy(malloc(strlen(path)+1), path);
    return font_resize(result, size, renderer);
}

void font_deinit(struct font *font)
{
    gm_deinit(&font->map);
    TTF_CloseFont(font->map.font);
    free(font->font_path);
    free(font);
}

struct font* font_resize(struct font *result, int size, SDL_Renderer *renderer)
{
    if (result->map.font)
        TTF_CloseFont(result->map.font);
    gm_deinit(&result->map);

    TTF_Font *font = TTF_OpenFont(result->font_path, size);
    TTF_SetFontHinting(font, TTF_HINTING_LIGHT);
    result->map = gm_init(font, 4);
    result->font_size = size;
    return result;
}

int font_get_size(struct font *font)
{
    return font->font_size;
}

int font_get_height(struct font *font)
{
    return font->map.height;
}

SDL_Point font_measure_glyph(struct font *font, int glyph)
{
    return gm_glyph_size(&font->map, glyph);
}

SDL_Point font_measure_text(struct font *font, const char *text)
{
    int width = 0;
    int max = strlen(text);
    int c;
    while ((c = utf8_get(&text, &max))) {
        width += font_measure_glyph(font, c).x;
    }
    return (SDL_Point) {width, font->map.height};
}

SDL_Point font_write_text(struct font *font, const char *text, SDL_Point xy, SDL_Renderer *renderer)
{
    SDL_Color c;
    SDL_GetRenderDrawColor(renderer, &c.r, &c.g, &c.b, &c.a);
    gm_set_color(&font->map, c);
    int orig = xy.x;
    int max = strlen(text);
    int ch;
    while ((ch = utf8_get(&text, &max))) {
        xy.x += gm_render_glyph(&font->map, xy, ch, renderer).x;
    }
    return (SDL_Point) { xy.x-orig, font->map.height };
}