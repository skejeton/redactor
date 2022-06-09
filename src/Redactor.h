#ifndef R_REDACTOR_H
#define R_REDACTOR_H

// Setup platform macros
#if defined(__linux__)
#   define Platform_Is_Linux
#elif defined(_WIN32)
#   define Platform_Is_Windows
#elif defined(__APPLE__) || defined(__MACH__)
#   define Platform_Is_Darwin
#elif defined(__unix__)
#   define Platform_Is_Unix
#else
#   define Platform_Is_Unknown
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "Buffer.h"
#include "Redex.h"
#include "Util.h"
#include "BufferDraw.h"
#include "Highlight.h"

#define Redactor_GlyphmapChunkMax 1024
#define Redactor_GlyphmapGlyphMax (Redactor_GlyphmapChunkMax * 256)

// Define macros here
#define DieErr(...) Util_DieErr(__VA_ARGS__)

#ifdef  __has_attribute
// Define attributes here
#endif

#define Bgm_Tiled 1

// Colors (NOTE: These are mostly for debugging)
#define Redactor_Color_Fore    (SDL_Color){229, 176, 131, 255}
#define Redactor_Color_String  (SDL_Color){186, 179, 73, 255}
#define Redactor_Color_Keyword (SDL_Color){102, 209, 145, 255}
#define Redactor_Color_Faded   (SDL_Color){135, 111, 89, 255}
#define Redactor_Color_Literal (SDL_Color){252, 156, 73, 255}
#define Redactor_Color_Call    (SDL_Color){150, 161, 125, 255}


struct {
    SDL_Rect glyphs[256];
    SDL_Texture *atlas;
}
typedef GlyphChunk;

struct {
    SDL_Texture *texture;
    int bgm_flags;
} 
typedef Background;

struct {
    bool ks_ctrl;
}
typedef Input;


struct Redactor {
    Background    toy_textureViewer_bg;
    float         toy_textureViewer_scale;

    char         *temp_respath;

    const char   *cfg_program_dataDir;
    const char   *cfg_font_respath;
    int           cfg_font_size;

    char         *program_location;
    char         *program_dataPath;
    bool          program_running;

    SDL_FPoint    render_scroll;
    SDL_Point     render_window_size;
    GlyphChunk   *render_font_chunks[Redactor_GlyphmapChunkMax];
    SDL_Window   *render_sdl_window;
    SDL_Renderer *render_sdl_renderer;
    TTF_Font     *render_sdl_font_handle;
    int           render_font_height;
    BufferDrawSegments render_drawSegments;

    bool          file_is_new;
    const char   *file_name;
    FILE         *file_handle;
    Buffer        file_buffer;
    Cursor        file_cursor;

    Input         input;
}
typedef Redactor;

// -- Background
void Background_Draw(Redactor *rs, Background *bg);

// -- Redactor
SDL_Point Redactor_DrawText(Redactor *rs, SDL_Color color, const char *text, int initx, int x, int y, int col);
int Redactor_Main(int argc, char *argv[]);

#endif
