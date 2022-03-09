#include "font.h"
#include "glyphmap.h"

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

int font_size(struct font *font)
{
    return font->font_size;
}

int utf8_get(const char **restrict s_, int *restrict max)
{
    const static int class[32] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5 };
    unsigned char *s = (unsigned char*)*s_;
    int c = 0;
    int cl = class[*s>>3];
    if (cl > *max) {
        return 0;
    }
    *max -= cl;
    switch (cl) {
        case 1:
            c = *s;
            break;
        case 2:
            c = (*s&0x1f)<<6;
            c |= (*++s&0x3f);
            break;
        case 3:
            c = (*s&0xf)<<12;
            c |= (*++s&0x3f)<<6;
            c |= (*++s&0x3f);
            break;
        case 4:
            c = (*s&0x7)<<18;
            c |= (*++s&0x3f)<<12;
            c |= (*++s&0x3f)<<6;
            c |= (*++s&0x3f);
            break;
    }
    *s_ = (char*)(s+!!c); 
    return c;
}

SDL_Point font_measure_text(struct font *font, const char *text)
{
    int width = 0;
    int max = strlen(text);
    while (*text) {
        // TODO: Unicode
        width += gm_query_metrics(&font->map, utf8_get(&text, &max)).advance;
    }
    return (SDL_Point) { width, font->map.height };
}

SDL_Point font_measure_glyph(struct font *font, int glyph)
{
    return gm_glyph_size(&font->map, glyph);
}




SDL_Point font_write_text(struct font *font, const char *text, SDL_Point xy, SDL_Renderer *renderer)
{
    SDL_Color c;
    SDL_GetRenderDrawColor(renderer, &c.r, &c.g, &c.b, &c.a);
    gm_set_color(&font->map, c);
    int orig = xy.x;
    int max = strlen(text);
    while (*text) {
        xy.x += gm_render_glyph(&font->map, xy, utf8_get(&text, &max), renderer).x;
    }
    return (SDL_Point) { xy.x-orig, font->map.height };
}