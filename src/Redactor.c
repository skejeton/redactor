// Setup platform macros
#if   defined(__linux__)
#       define Platform_Is_Linux
#elif defined(_WIN32)
#       define Platform_Is_Windows
#elif defined(__APPLE__) || defined(__MACH__)
#       define Platform_Is_Darwin
#elif defined(__unix__)
#       define Platform_Is_Unix
#else
#       define Platform_Is_Unknown
#endif


// Put includes here
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef Platform_Is_Linux
#include <unistd.h>
#include <libgen.h>
#endif

#include "Unicode.h"

// Define macros here
#define DieErr(...) do {fprintf(stderr, __VA_ARGS__); exit(-1);} while (0)

#ifdef  __has_attribute
// Define attributes here
#endif

typedef struct {
        SDL_Rect glyphs[256];
        SDL_Texture *atlas;
} GlyphChunk;

#define Bgm_Tiled       1<<0

typedef struct {
        SDL_Texture *texture;
        int bgm_flags;
} Background;

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

        GlyphChunk    render_font_ascii_chunk;
        SDL_Window   *render_sdl_window;
        SDL_Renderer *render_sdl_renderer;
        TTF_Font     *render_sdl_font_handle;

        bool          file_is_new;
        const char   *file_name;
        FILE         *file_handle;
        char         *file_data;
}
typedef Redactor;

char *Util_ReadFileStr(FILE *f)
{
        char *s;
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        fseek(f, 0, SEEK_SET);
        s = malloc(size + 1);
        s[fread(s, 1, size, f)] = 0;
        return s;
}

// Returns malloc'd string
char *Util_GetProgramPath()
{
        const size_t PathMax = 0xFFF;
        const size_t ByteSize = sizeof(char) * PathMax + 1;
        char *path = malloc(ByteSize);
#if defined(Platform_Is_Linux)
        // NOTE: Redundant allocation is needed because
        //       string returned by dirname can be overwritten at any time.
        //       I can try to avoid it but it doesn't matter that much.
        //       The problem is that it //MAY// modify path instead of using a static buffer.
        char *filepath  = malloc(sizeof(char) * PathMax + 1);
        // TODO: Handle path that's more than PathMax.
        //       How would I detect that?
        int written_chars = readlink("/proc/self/exe", filepath, PathMax);
        if (written_chars == -1) {
                DieErr("Fatal: Failed to retrieve process path: %s", strerror(errno));
        }
        // NOTE: Readlink returns the program path including the name,
        //       I don't need that.
        char *dirpath = dirname(filepath);
        int i;
        for (i = 0; i < PathMax && dirpath[i]; ++i)
                path[i] = dirpath[i];
        path[i] = 0;
        free(filepath);
#else
#       error "I can't get program path for this platform"
#endif
        return path;
}

char *Util_ConcatPaths(const char *path_a, const char *path_b)
{
        int la = strlen(path_a), lb = strlen(path_b);
        int len = la + 1 + lb;
        char *s = malloc(len + 1);
        memcpy(s, path_a, la);
#if defined(Platform_Is_Windows) 
        s[la] = '\\';
#elif !defined(Platform_Is_Unknown)
        s[la] = '/';
#else
#       error "I don't know the path separator for this platform"
#endif
        s[len] = 0;
        memcpy(s+la+1, path_b, lb);
        return s;
}


// -- util

char *Redactor_GetTempResPath(Redactor *rs, const char *resname)
{
        free(rs->temp_respath);
        return (rs->temp_respath = Util_ConcatPaths(rs->program_dataPath, resname));
}

void Redactor_PrintMeta(Redactor *rs)
{
        printf("|-- redactor meta -------------\n");
        printf("|   cfg_program_dataDir  | %s\n", rs->cfg_program_dataDir);
        printf("|   cfg_font_respath     | %s\n", rs->cfg_font_respath);
        printf("|   cfg_font_size        | %d\n", rs->cfg_font_size);
        printf("|   program_location     | %s\n", rs->program_location);
        printf("|   program_dataPath     | %s\n", rs->program_dataPath);
        printf("|   file_name            | %s\n", rs->file_name);
        printf("|   file_is_new          | %d\n", rs->file_is_new);
        printf("|-- file_data -----------------\n%s\n", rs->file_data);
        printf("|-- end redactor meta ---------\n");
}

void Redactor_PackAsciiCharTab(Redactor *rs)
{
        int sfw = 300, sfh = 300, x = 0, y = 0, maxh = 0, padding = 0;
        SDL_Surface *dsf = SDL_CreateRGBSurface(0, sfw, sfh, 32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000);
        SDL_SetSurfaceBlendMode(dsf, SDL_BLENDMODE_BLEND);
        SDL_FillRect(dsf, NULL, SDL_MapRGBA(dsf->format, 0, 0, 0, 255));

        

        for (int i = 0; i < 256; ++i) {
                SDL_Surface *chsf = TTF_RenderGlyph32_Blended(rs->render_sdl_font_handle, i, (SDL_Color){255, 255, 255, 255});
        
                if (chsf) {
                        // NOTE: Destination only
                        int chsfw = chsf->w+padding*2;
                        int chsfh = chsf->h+padding*2;

                        // NOTE: Check if can't fit in row anymore
                        while (chsfw > (sfw-x)) {
                                x = 0;
                                y += maxh;
                                maxh = chsfh;
                        }

                        // NOTE: Check if can't fit in column anymore (means we cant fit all chars -- total fail)
                        if (chsfh > (sfh-y)) {
                                // TODO: Instead resize surface and try again
                                DieErr("Too tight to pack all ascii chars");
                        }

                        if (chsfh > maxh) {
                                maxh = chsfh;
                        }

                        SDL_Rect dest = (SDL_Rect){x+padding, y+padding, chsf->w, chsf->h};
                        rs->render_font_ascii_chunk.glyphs[i] = dest;
                        SDL_BlitSurface(chsf, &(SDL_Rect){0, 0, chsf->w, chsf->h}, dsf, &dest);
                        x += chsfw;

                        SDL_FreeSurface(chsf);
                }
        }

        rs->render_font_ascii_chunk.atlas = SDL_CreateTextureFromSurface(rs->render_sdl_renderer, dsf);
        SDL_FreeSurface(dsf);
}

// -- init/deinit

void Redactor_Init(Redactor *rs)
{
        // -- Init default cfg 
        rs->cfg_program_dataDir = "data";
        rs->cfg_font_respath = "monospace.ttf";
        rs->cfg_font_size = 16;

        // -- Init paths
        rs->program_location = Util_GetProgramPath();
        rs->program_dataPath = Util_ConcatPaths(rs->program_location, rs->cfg_program_dataDir);

        // -- Init sdl
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
                DieErr("Fatal: Can not init SDL: %s\n", SDL_GetError());
        }

        if (TTF_Init() < 0) {
                DieErr("Fatal: Can not init SDL_ttf: %s\n", TTF_GetError());
        }

        rs->render_sdl_window = SDL_CreateWindow("redactor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920/2, 1080/2, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!rs->render_sdl_window) {
                DieErr("Fatal: Can not init window: %s\n", SDL_GetError());
        }
                
        rs->render_sdl_renderer = SDL_CreateRenderer(rs->render_sdl_window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!rs->render_sdl_renderer) {
                DieErr("Fatal: Can not init renderer: %s\n", SDL_GetError());
        }

        SDL_SetRenderDrawBlendMode(rs->render_sdl_renderer, SDL_BLENDMODE_BLEND);

        // -- Init font
        rs->render_sdl_font_handle = TTF_OpenFont(Redactor_GetTempResPath(rs, rs->cfg_font_respath), rs->cfg_font_size);
        if (!rs->render_sdl_font_handle) {
                DieErr("Fatal: Can not font: %s\n", TTF_GetError());
        }

        // -- Init values
        rs->program_running = true;
        rs->toy_textureViewer_scale = 1;

        // TODO: Handle loading resources better
        SDL_Surface *bgSurface = SDL_LoadBMP(Redactor_GetTempResPath(rs, "debgtool.bmp"));
        rs->toy_textureViewer_bg.texture = SDL_CreateTextureFromSurface(rs->render_sdl_renderer, bgSurface);
        SDL_FreeSurface(bgSurface);
        rs->toy_textureViewer_bg.bgm_flags = 1;

        // -- Misc
        Redactor_PackAsciiCharTab(rs);
}

void Redactor_UseArgs(Redactor *rs, int argc, char *argv[])
{
        if (argc != 2) {
                DieErr("Usage: %s file.txt\n", argv[0]);
        }

        rs->file_name = argv[1];
        rs->file_handle = fopen(rs->file_name, "r+");
        if (!rs->file_handle) {
                // TODO: This will create upon opening it,
                //       I should probably create it after write.
                rs->file_handle = fopen(rs->file_name, "w+");
                if (!rs->file_handle) {
                        DieErr("Fatal: Error opening file %s: %s\n", rs->file_name, strerror(errno));
                } else {
                        rs->file_is_new = true;
                }
        }
        rs->file_data = Util_ReadFileStr(rs->file_handle);
        
}

void Redactor_End(Redactor *rs)
{
        // NOTE: Allocated in UseArgs
        free(rs->file_data);
        // NOTE: Allocated in GetTempResPath
        free(rs->temp_respath);
        // NOTE: Allocated in SetupPaths
        free(rs->program_location);
        free(rs->program_dataPath);

        TTF_CloseFont(rs->render_sdl_font_handle);
        SDL_DestroyTexture(rs->toy_textureViewer_bg.texture);
        SDL_DestroyTexture(rs->render_font_ascii_chunk.atlas);
        SDL_DestroyRenderer(rs->render_sdl_renderer);
        SDL_DestroyWindow(rs->render_sdl_window); 
        fclose(rs->file_handle);
        TTF_Quit();
        SDL_Quit();
}

// -- draw

int Redactor_DrawText(Redactor *rs, int x, int y, const char *text)
{
        int y_delta = 0, c;
        

        while (c = Uni_Utf8_NextVeryBad(&text)) {
                if (c < 0 || c >= 256) {
                        continue;
                        // TODO: Print invalid char code
                }

                SDL_Rect src = rs->render_font_ascii_chunk.glyphs[c];
                SDL_RenderCopy(rs->render_sdl_renderer, rs->render_font_ascii_chunk.atlas, &src, &(SDL_Rect){x, y, src.w, src.h});
                y_delta = src.h;
                x += src.w;
        }

        return y_delta;
}

void Redactor_DrawDocument(Redactor *rs)
{
        char *buffer = rs->file_data;
        int y = 0;
        while (*buffer) {
                const char *start = buffer;
                char c;

                // Step until newline
                while (*buffer && *buffer != '\n') {
                        buffer++;
                }

                c = *buffer;
                *buffer = 0;

                y += Redactor_DrawText(rs, 0, y, start);

                *buffer = c;

                // NOTE: Skip the newline unless it's null terminator
                if (*buffer) {
                        buffer++;
                }

        }
}

void Redactor_DrawBg(Redactor *rs, Background *bg)
{
        int ofsx = 0, ofsy = 0, xcount = 0, ycount = 0;
        int texture_w, texture_h;
        SDL_QueryTexture(bg->texture, NULL, NULL, &texture_w, &texture_h);

        // NOTE: Zero divide
        if (texture_w == 0 || texture_h == 0)  {
                return;
        }

        if (bg->bgm_flags | Bgm_Tiled) {
                int screen_w, screen_h;
                SDL_GetWindowSize(rs->render_sdl_window, &screen_w, &screen_h);
                xcount = screen_w / texture_w + 2;
                ycount = screen_h / texture_h + 2;
        } 
 
        for (int x = 0; x < xcount; ++x) {
                for (int y = 0; y < ycount; ++y) {
                        SDL_RenderCopy(rs->render_sdl_renderer, bg->texture, NULL, &(SDL_Rect){x*texture_w, y*texture_w, texture_w, texture_h});
                }
        }
}

// NOTE: For debugging
void Redactor_DrawTextureViewer(Redactor *rs, SDL_Texture *texture)
{
        float scale = rs->toy_textureViewer_scale;
        int texture_w, texture_h, screen_w, screen_h, tex_pos_x, tex_pos_y;
        SDL_QueryTexture(texture, NULL, NULL, &texture_w, &texture_h);
        SDL_GetWindowSize(rs->render_sdl_window, &screen_w, &screen_h);
        
        texture_w *= scale;
        texture_h *= scale;

        tex_pos_x = (screen_w - texture_w) / 2;
        tex_pos_y = (screen_h - texture_h) / 2;

        SDL_SetRenderDrawColor(rs->render_sdl_renderer, 0, 0, 80, 255);
        Redactor_DrawBg(rs, &rs->toy_textureViewer_bg);
        char title[1024];
        snprintf(title, 1024, "Texture viewer | w %d | h %d | s %g", texture_w, texture_h, scale);

        Redactor_DrawText(rs, tex_pos_x, tex_pos_y-20, title);
        SDL_SetRenderDrawColor(rs->render_sdl_renderer, 70, 50, 128, 128);
        SDL_RenderDrawRect(rs->render_sdl_renderer, &(SDL_Rect){tex_pos_x-2, tex_pos_y-2, texture_w+4, texture_h+4});
        SDL_RenderCopy(rs->render_sdl_renderer, texture, NULL, &(SDL_Rect){tex_pos_x, tex_pos_y, texture_w, texture_h});
}

// -- control


void Redactor_HandleEvents(Redactor *rs)
{
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_QUIT:
                        rs->program_running = false;
                        break;
                case SDL_MOUSEWHEEL:
                        rs->toy_textureViewer_scale += event.wheel.y/10.0;
                        break;
                }
        }
}

void Redactor_Cycle(Redactor *rs)
{
        Redactor_HandleEvents(rs);
        SDL_SetRenderDrawColor(rs->render_sdl_renderer, 0, 0, 0, 255);
        SDL_RenderClear(rs->render_sdl_renderer);
        Redactor_DrawDocument(rs);
       // Redactor_DrawTextureViewer(rs, rs->render_font_ascii_chunk.atlas);
        
        SDL_RenderPresent(rs->render_sdl_renderer);
}

int Redactor_Main(int argc, char *argv[])
{
        Redactor rs = {0};
        Redactor_Init(&rs);

        Redactor_UseArgs(&rs, argc, argv);
        Redactor_PrintMeta(&rs);
        while (rs.program_running) {
                Redactor_Cycle(&rs);
        }
        Redactor_End(&rs);
}
