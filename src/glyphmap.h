#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include "rect.h"
#define GLYPHS_PER_PAGE 256

struct glyphmetrics {
    // Glyph local offset and maximum offset, you can deduce the width or height here
    int minx, miny;
    int maxx, maxy;
    // This is by how much you should move your glyph in the X direction
    int advance;
};

struct glyphdat {
    // NOTE: This is the position in the texture provided in the texture in glyph chunk
    SDL_Rect cutout;
    struct glyphmetrics metrics;
};

struct glyphchunk {
    // NOTE: This has size of GLYPHS_PER_PAGE
    struct glyphdat *data;
    SDL_Point pack_bounds;
    SDL_Texture *texture;
};

struct glyphmap {
    // This is enough to cover more than every unicode character,
    // the glyph chunk size is 256 glyphs, unicode has total of
    // 144,697 characters (as of unicode 14.0), this leaves
    // us with additional 117447 characters possible for future use
    struct glyphchunk chunks[1024];
    // Padding around characters, this is needed to reduce artifacts
    int padding;
    // The font itself
    TTF_Font *font;
    // This is the font height for consistency
    int height;
    // Color key
    SDL_Color color;
};

// Initializes font with specified padding around textures
// ***The TTF_Font will not be managed or freed by glyphmap***
static struct glyphmap gm_init(TTF_Font *font, int padding);
// I'm reminding again, ***The TTF_Font will not be freed here***
static void gm_deinit(struct glyphmap *glyphmap);
// Queries metrics of specific character
static struct glyphmetrics gm_query_metrics(struct glyphmap *map, int character);
// Returns size with correct kerning
SDL_Point gm_glyph_size(struct glyphmap *map, int character);
// Renders a glyph on screen, returns size with correct kerning
static SDL_Point gm_render_glyph(struct glyphmap *map, SDL_Point at, int character, SDL_Renderer *renderer);
// Returns rectangle positioned at minx,miny with size of the glyph rectangle
// NOTE: This shouldn't be used as a guideline for rendering text, as this is not positioned or aligned properly
//       at all, moreover this doesn't store the "advance" information
static SDL_Rect gm_glyph_raw_rect(struct glyphmetrics metrics);



static struct glyphmap gm_init(TTF_Font *font, int padding)
{
    struct glyphmap result = { 0 };
    result.padding = padding;
    result.font = font;
    result.height = TTF_FontHeight(font);
    result.color = (SDL_Color){255, 255, 255, 255};
    return result;
}

static void free_chunk_(struct glyphchunk *chunk)
{
    SDL_DestroyTexture(chunk->texture);
    free(chunk->data);
}

static void gm_deinit(struct glyphmap *glyphmap)
{
    // Free every single chunk
    for (int i = 0; i < 1024; ++i) {
        free_chunk_(glyphmap->chunks+i);
    }
}

static struct glyphmetrics construct_glyph_metrics_(TTF_Font *font, int glyphNo)
{
    struct glyphmetrics result = {0};
    if (TTF_GlyphMetrics32(font, glyphNo, &result.minx, &result.maxx, &result.miny, &result.maxy, &result.advance))
        // Return stub, 1x1 to avoid division problems. 
        // TODO: Special fallback glyph
        result = (struct glyphmetrics){0, 0, 1, 1, 1};

    result.miny = 0; 
    result.maxy = TTF_FontHeight(font);
    printf("metrics '%c' %d %d %d %d\n", glyphNo, result.minx, result.miny, result.maxx, result.maxy);
    return result;
}

static SDL_Rect gm_glyph_raw_rect(struct glyphmetrics metrics)
{
    return (SDL_Rect){metrics.minx, metrics.miny, metrics.maxx-metrics.minx, metrics.maxy-metrics.miny};
}

static int try_pack_glyphs_(struct glyphchunk *chunk, int padding, SDL_Point bounds)
{
    SDL_Point cursor = {0, 0};
    int max_height = 0;
    for (int i = 0; i < GLYPHS_PER_PAGE; ++i) {
        int remaining_height = bounds.y - cursor.y;
        int remaining_width = bounds.x - cursor.x;
        SDL_Rect *glyph_cutout = &chunk->data[i].cutout;
        struct glyphmetrics metrics = chunk->data[i].metrics;
        *glyph_cutout = gm_glyph_raw_rect(metrics);
        SDL_Rect glyph_rect = rect_inset(*glyph_cutout, -padding);
        
        max_height = max_height < glyph_rect.h ? glyph_rect.h : max_height;

        while (glyph_rect.w > remaining_width) {
            cursor.x = 0;
            cursor.y += max_height;
            max_height = glyph_rect.h;
            remaining_height = bounds.y - cursor.y;
            remaining_width = bounds.x - cursor.x;
        }

        // If there's not enough height we can't fit everything
        if (glyph_rect.h > remaining_height) {
            return -1;
        }

        glyph_cutout->x = cursor.x+padding;
        glyph_cutout->y = cursor.y+padding;

        cursor.x += glyph_rect.w;
    }
    return 0;
}

static void pack_glyphs_(struct glyphchunk *chunk, int padding)
{
    // 256 x 256 should be reasonable for smaller font sizes,
    // reducing the memory usage
    SDL_Point bounds = { 256, 256 };

    // Try to pack until it fits
    while (try_pack_glyphs_(chunk, padding, bounds)) {
        bounds.x *= 2;
        bounds.y *= 2;
    }

    chunk->pack_bounds = bounds;
}

// This function does not initialize the texture and does not do rectangle packing!
static struct glyphchunk* init_chunk_metrics_(struct glyphchunk *destination, TTF_Font *font, int padding, int page)
{
    destination->data = calloc(GLYPHS_PER_PAGE, sizeof(struct glyphdat));

    int starting_char_id = page*GLYPHS_PER_PAGE;

    for (int i = 0; i < GLYPHS_PER_PAGE; ++i) {
        destination->data[i].metrics = construct_glyph_metrics_(font, i+starting_char_id);
    } 

    pack_glyphs_(destination, padding);

    return destination;
}

static struct glyphchunk* query_chunk_with_metrics_(struct glyphmap *map, int character)
{
    if (character < 0)
        return NULL;

    int page = character/GLYPHS_PER_PAGE;

    if (page < 0 || page >= 1024) {
        return NULL;
    }

    struct glyphchunk *chunk = &map->chunks[page];

    // Initialize chunk metrics if not initialized
    if (chunk->data == NULL) {
        return init_chunk_metrics_(chunk, map->font, map->padding, page);
    } else {
        return chunk;
    }
}

static struct glyphmetrics gm_query_metrics(struct glyphmap *map, int character)
{
    struct glyphchunk *chunk = query_chunk_with_metrics_(map, character);
    if (chunk) {
        return chunk->data[character % GLYPHS_PER_PAGE].metrics;
    } else {
        return (struct glyphmetrics){0, 0, 1, 1, 1};
    }
}

static struct glyphchunk* init_chunk_texture_(struct glyphchunk *destination, TTF_Font *font, int page, SDL_Renderer *renderer)
{
    const SDL_Color BLACK = {0, 0, 0, 255};

    SDL_Surface *surface = SDL_CreateRGBSurface(0, destination->pack_bounds.x, destination->pack_bounds.y, 32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
    SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0, 0, 0, 0));

    int starting_char_id = page*GLYPHS_PER_PAGE;


    for (int i = 0; i < GLYPHS_PER_PAGE; ++i) {
        SDL_Rect cutout = destination->data[i].cutout;
        SDL_Rect source = gm_glyph_raw_rect(destination->data[i].metrics);
        SDL_Surface *glyph_surface;
        if ((glyph_surface = TTF_RenderGlyph32_Blended(font, starting_char_id+i, BLACK))) {
            SDL_BlitSurface(glyph_surface, &source, surface, &cutout);
            SDL_FreeSurface(glyph_surface);
        }
    }

    // I can not believe i have to do this! 
    // RenderGlyph32_Blended gives very jagged glyph texture
    // But for some reason it doesn't do it for the black color
    // So I just invert it
    SDL_LockSurface(surface);
    for (int x = 0; x < surface->w; ++x) {
        for (int y = 0; y < surface->h; ++y) {
            int i = x * 4 + y * surface->pitch;
            ((unsigned char*)surface->pixels)[i] = 255;
            ((unsigned char*)surface->pixels)[i+1] = 255;
            ((unsigned char*)surface->pixels)[i+2] = 255;
        }
    }
    SDL_UnlockSurface(surface);

    destination->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (destination->texture == NULL) {
        fprintf(stderr, "%s %d %d\n", "Failed to create font texture of size\n", destination->pack_bounds.x, destination->pack_bounds.y);
        exit(-1);
    }
    SDL_FreeSurface(surface);
    return destination;
}

static struct glyphchunk* query_chunk_with_texture_(struct glyphmap *map, int character, SDL_Renderer *renderer)
{
    struct glyphchunk *chunk = query_chunk_with_metrics_(map, character);
    
    if (chunk == NULL) 
        return NULL;
    
    int page = character/GLYPHS_PER_PAGE;

    // Initialize chunk metrics if not initialized
    if (chunk->texture == NULL) {
        return init_chunk_texture_(chunk, map->font, page, renderer);
    } else {
        return chunk;
    }
}

void gm_set_color(struct glyphmap *map, SDL_Color color)
{
    map->color = color;
}

SDL_Point gm_glyph_size(struct glyphmap *map, int character)
{
    return (SDL_Point){gm_query_metrics(map, character).advance, map->height};
}

static SDL_Point gm_render_glyph(struct glyphmap *map, SDL_Point at, int character, SDL_Renderer *renderer)
{
    struct glyphchunk *chunk = query_chunk_with_texture_(map, character, renderer);
    if (chunk) {
        // TODO: do a check instead of querying SDL every time
        SDL_SetTextureColorMod(chunk->texture, map->color.r, map->color.g, map->color.b);
        SDL_SetTextureAlphaMod(chunk->texture, map->color.a);
        struct glyphdat *glyph = &chunk->data[character%GLYPHS_PER_PAGE];
        struct glyphmetrics m = glyph->metrics;
        SDL_RenderCopy(renderer, chunk->texture, &glyph->cutout,
                       &(SDL_Rect){at.x+m.minx, at.y+m.miny, m.maxx-m.minx, m.maxy-m.miny});
        return (SDL_Point){m.advance, map->height};
    } else {
        return (SDL_Point){1, 1};
    }
}