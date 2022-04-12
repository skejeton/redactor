#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#define Redactor_GlyphmapChunkMax 1024
#define Redactor_GlyphmapGlyphMax (Redactor_GlyphmapChunkMax * 256)

// Define macros here
#define DieErr(...) do {fprintf(stderr, __VA_ARGS__); exit(-1);} while (0)

#ifdef  __has_attribute
// Define attributes here
#endif

#define Bgm_Tiled 1

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
        // NOTE: text is a utf8 string
        char *text;
        // NOTE: text_size is the size in bytes
        size_t text_size;
        // NOTE: text_len is the size in runes
        size_t text_len;
}
typedef Line;

struct {
        int32_t line;
        int32_t column;
} 
typedef Cursor;

struct {
        Line *lines;
        size_t lines_len;
}
typedef Buffer;

struct {
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

        bool          file_is_new;
        const char   *file_name;
        FILE         *file_handle;
        Buffer        file_buffer;
        Cursor        file_cursor;
}
typedef Redactor;


// -- Background
void Background_Draw(Redactor *rs, Background *bg);

// -- Redactor
int Redactor_Main(int argc, char *argv[]);
