#include "font.h"
#include <SDL2/SDL_ttf.h>

struct font {
    char *path;
    int size;
    struct font_glyph {
        SDL_Texture *texture;
        SDL_Surface *surface;
        int w, h;
    } glyphs[256];
};

int font_size(struct font *font)
{
    return font->size;
}

static void destroy_glyphs(struct font *font)
{
    // HACK: Checking if it's a first initialization
    if (font->glyphs[0].surface == NULL)
        return;
    for (int i = 0; i < 256; i++) {
        SDL_DestroyTexture(font->glyphs[i].texture);
        SDL_FreeSurface(font->glyphs[i].surface);
    }
}

struct font* font_resize(struct font *result, int size, SDL_Renderer *renderer)
{
    destroy_glyphs(result);
    result->size = size;
    TTF_Font *font = TTF_OpenFont(result->path, size);
    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);
    if (font == NULL)
        return NULL;

    for (int i = 0; i < 256; i += 1) {
        SDL_Surface *text_surface = TTF_RenderGlyph32_Blended(font, i, (SDL_Color) { 255, 255, 255, 255 });
        int h, w;
        if (text_surface != NULL) {
            h = text_surface->h;
            w = text_surface->w;
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            result->glyphs[i] = (struct font_glyph) {
                .h = h, .w = w,
                .texture = texture, .surface = text_surface
            };
        }
    }

    TTF_CloseFont(font);
    return result;
}

struct font* font_init(const char *path, int size, SDL_Renderer *renderer)
{
    struct font *result = calloc(sizeof(struct font), 1);
    result->path = strcpy(malloc(strlen(path)+1), path);
    return font_resize(result, size, renderer);
}


SDL_Point font_measure_glyph(int glyph, struct font *font)
{
    if (glyph >= 0 && glyph <= 255) {
        return (SDL_Point) {font->glyphs[glyph].w, font->glyphs[glyph].h};
    } else {
        fprintf(stderr, "Unsupported glyph recieved: %d (TODO unicode handling)\n", glyph);
        return font_measure_glyph(1, font);
    }
}


SDL_Point font_measure_text(const char *text, struct font *font)
{
    SDL_Point result = {0, 0};
    
    // Make blank characters to have a height
    if (!*text)
        return (SDL_Point) { 0, font->glyphs[' '].h }; 

    while (*text) {
        struct font_glyph glyph = font->glyphs[*text++];
        result.x += glyph.w;
        result.y = result.y < glyph.h ? glyph.h : result.y; // max
    }
    
    return result;
}


SDL_Point font_write_text(const char *text, SDL_Point xy, SDL_Renderer *renderer, struct font *font)
{
    struct font_glyph *glyphs = font->glyphs; 
    unsigned char r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    int rw = 0;
    if (!*text) { rw = 1; text = " "; };
    int w = 0, h = 0;
    for (;*text;text++) {
        struct font_glyph glyph = font->glyphs[*text];
        SDL_SetTextureColorMod(glyph.texture, r, g, b);
        SDL_SetTextureAlphaMod(glyph.texture, a);
        h = h > glyphs[*text].h ? h : glyphs[*text].h;
        SDL_RenderCopy(renderer, glyph.texture, NULL, &(SDL_Rect){ xy.x+w, xy.y, glyph.w, glyph.h });
        if (!rw)
            w += glyphs[*text].w;

    }
    return (SDL_Point){w, h};
}


void font_deinit(struct font *font)
{
    destroy_glyphs(font);
    free(font->path);
    free(font);
}
