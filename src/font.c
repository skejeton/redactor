#include "font.h"
#include "rectpack.h"
#include <SDL2/SDL_ttf.h>

// Group of 256 glyphs
struct font_glyph_group {
    struct rectpack pack;
    SDL_Texture *texture;
};

struct font {
    struct font_glyph_group ascii;
    char *path;
    int size;
};

int font_size(struct font *font)
{
    return font->size;
}

static void destroy_glyphs(struct font *font)
{
    // HACK: Checking if it's a first initialization
    rectpack_deinit(&font->ascii.pack);
    SDL_DestroyTexture(font->ascii.texture);
}

struct font* font_resize(struct font *result, int size, SDL_Renderer *renderer)
{
    result->ascii.pack = rectpack_init();
    destroy_glyphs(result);
    result->size = size;
    TTF_Font *font = TTF_OpenFont(result->path, size);
    TTF_SetFontHinting(font, TTF_HINTING_NORMAL);
    if (font == NULL)
        return NULL;

    for (int i = 0; i < 256; i += 1) {
        int minx, maxx, miny, maxy, advance;
        SDL_Surface *text_surface = TTF_RenderGlyph32_Blended(font, i, (SDL_Color) { 255, 255, 255, 255 });
        if (TTF_GlyphMetrics32(font, i, &minx, &maxx, &miny, &maxy, &advance)) {
            fprintf(stderr, "GLYPH FAIL U+%04x\n", i);
            minx = maxx = miny = maxy = advance = 0; 
        }
        if (text_surface != NULL) {
            int h = text_surface->h;
            int w = text_surface->w;
            SDL_FreeSurface(text_surface);
            rectpack_add(&result->ascii.pack, (SDL_Point){w, h}, 1);
        } else {
            rectpack_add(&result->ascii.pack, (SDL_Point){0, 0}, 1);
        }
    }

    // TODO: Check for error
    if (rectpack_fit(&result->ascii.pack, (SDL_Point){1024, 1024}) != 256)
        fprintf(stderr, "Couldn't fit all characters!\n");

    SDL_Texture *atlas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1024, 1024);
    SDL_SetRenderTarget(renderer, atlas);

    for (int i = 0; i < 256; i += 1) {
        SDL_Rect rect = result->ascii.pack.rects[i].rect;
        printf("Rect is %d %d %d %d\n", rect.x, rect.y, rect.w, rect.h);
        SDL_Surface *text_surface = TTF_RenderGlyph32_Blended(font, i, (SDL_Color) { 255, 255, 255, 255 });
        // TODO: Check for error
        if (text_surface != NULL) {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            // TODO: Check for error
            if (texture != NULL) {
                SDL_RenderCopy(renderer, texture, NULL, &rect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(text_surface);
        }
    }

    result->ascii.texture = atlas;

    SDL_SetRenderTarget(renderer, NULL);

    TTF_CloseFont(font);
    return result;
}

struct font* font_init(const char *path, int size, SDL_Renderer *renderer)
{
    struct font *result = calloc(sizeof(struct font), 1);
    result->path = strcpy(malloc(strlen(path)+1), path);
    return font_resize(result, size, renderer);
}

static SDL_Rect get_glyph_rect(int glyph, struct font *font)
{
    return font->ascii.pack.rects[glyph].rect;
}

SDL_Point font_measure_glyph(int glyph, struct font *font)
{
    if (glyph >= 0 && glyph <= 255) {
        SDL_Rect r = get_glyph_rect(glyph, font);
        return (SDL_Point) {r.w, r.h};
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
        return (SDL_Point) { 0, get_glyph_rect(' ', font).h }; 

    while (*text) {
        SDL_Rect glyph = get_glyph_rect(*text++, font);
        result.x += glyph.w;
        result.y = result.y < glyph.h ? glyph.h : result.y; // max
    }
    
    return result;
}


SDL_Point font_write_text(const char *text, SDL_Point xy, SDL_Renderer *renderer, struct font *font)
{
    unsigned char r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    int rw = 0;
    if (!*text) { rw = 1; text = " "; };
    int w = 0, h = 0;
    for (;*text;text++) {
        SDL_Rect glyph = get_glyph_rect(*text, font);
        SDL_SetTextureColorMod(font->ascii.texture, r, g, b);
        SDL_SetTextureAlphaMod(font->ascii.texture, a);
        h = h > glyph.h ? h : glyph.h;
        SDL_RenderCopy(renderer, font->ascii.texture, &glyph, &(SDL_Rect){ xy.x+w, xy.y, glyph.w, glyph.h });
        if (!rw)
            w += glyph.w;

    }
    return (SDL_Point){w, h};
}


void font_deinit(struct font *font)
{
    destroy_glyphs(font);
    free(font->path);
    free(font);
}
