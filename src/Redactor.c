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

// Define macros here
#define DieErr(...) do {fprintf(stderr, __VA_ARGS__); exit(-1);} while (0)

#ifdef  __has_attribute
// Define attributes here
#endif

struct {
        char         *temp_respath;

        const char   *cfg_program_dataDir;
        const char   *cfg_font_respath;
        int           cfg_font_size;

        char         *program_location;
        char         *program_dataPath;
        bool          program_running;

        SDL_Window   *sdl_window;
        SDL_Renderer *sdl_renderer;

        TTF_Font     *sdl_font_handle;

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

        rs->sdl_window = SDL_CreateWindow("redactor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920/2, 1080/2, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!rs->sdl_window) {
                DieErr("Fatal: Can not init window: %s\n", SDL_GetError());
        }
                
        rs->sdl_renderer = SDL_CreateRenderer(rs->sdl_window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!rs->sdl_renderer) {
                DieErr("Fatal: Can not init renderer: %s\n", SDL_GetError());
        }

        // -- Init font
        rs->sdl_font_handle = TTF_OpenFont(Redactor_GetTempResPath(rs, rs->cfg_font_respath), rs->cfg_font_size);
        if (!rs->sdl_font_handle) {
                DieErr("Fatal: Can not font: %s\n", TTF_GetError());
        }

        // -- Init values
        rs->program_running = true;
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

        TTF_CloseFont(rs->sdl_font_handle);
        SDL_DestroyRenderer(rs->sdl_renderer);
        SDL_DestroyWindow(rs->sdl_window); 
        fclose(rs->file_handle);
        TTF_Quit();
        SDL_Quit();
}

// -- draw

int Redactor_DrawText(Redactor *rs, int x, int y, const char *text)
{
        SDL_Surface *text_surface = TTF_RenderUTF8_Blended(rs->sdl_font_handle, text, (SDL_Color){255, 255, 255, 255});
        if (text_surface == NULL) {
                return 0; // TODO: Log failed text allocation
        }

        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(rs->sdl_renderer, text_surface);
        if (text_texture == NULL) {
                SDL_FreeSurface(text_surface);
                return 0; // TODO: Log failed texture allocation
        }

        int y_delta = text_surface->h;

        // -- draw
        SDL_RenderCopy(rs->sdl_renderer, text_texture, NULL, &(SDL_Rect){x, y, text_surface->w, text_surface->h});

        // -- free
        SDL_DestroyTexture(text_texture);
        SDL_FreeSurface(text_surface);

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

// -- control

void Redactor_HandleEvents(Redactor *rs)
{
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_QUIT:
                        rs->program_running = false;
                        break;
                }
        }
}

void Redactor_Cycle(Redactor *rs)
{
        Redactor_HandleEvents(rs);
        SDL_SetRenderDrawColor(rs->sdl_renderer, 0, 0, 0, 255);
        SDL_RenderClear(rs->sdl_renderer);
        Redactor_DrawDocument(rs);
        
        SDL_RenderPresent(rs->sdl_renderer);
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
