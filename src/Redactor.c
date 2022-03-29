// Put includes here
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

// Define macros here
#define DieErr(...) do {fprintf(stderr, __VA_ARGS__); exit(-1);} while (0)

#ifdef  __has_attribute
// Define attributes here
#endif

struct {
        bool          program_running;

        SDL_Window   *sdl_window;
        SDL_Renderer *sdl_renderer;

        const char   *file_name;
        FILE         *file_handle;
        char         *file_data;
} 
typedef Redactor;

char* Util_ReadFileStr(FILE *f)
{
        char *s;
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        fseek(f, 0, SEEK_SET);
        s = malloc(size + 1);
        s[fread(s, 1, size, f)] = 0;
        return s;
}

void Redactor_InitWin(Redactor *rs)
{
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

        rs->program_running = true;
}

// Checks out the arguments and sets needed values
void Redactor_UseArgs(Redactor *rs, int argc, char *argv[])
{
        if (argc != 2) {
                DieErr("Usage: %s file.txt\n", argv[0]);
        }

        rs->file_name = argv[1];
        rs->file_handle = fopen(rs->file_name, "rw");
        if (!rs->file_handle) {
                DieErr("Fatal: Error opening file %s: %s\n", rs->file_name, strerror(errno));
        }
        rs->file_data = Util_ReadFileStr(rs->file_handle);
        
}
 

void Redactor_EndSDL(Redactor *rs)
{
        TTF_Quit();
        SDL_Quit();
}

void Redactor_End(Redactor *rs)
{
        // NOTE: Allocated in UseArgs
        free(rs->file_data);
        SDL_DestroyRenderer(rs->sdl_renderer);
        SDL_DestroyWindow(rs->sdl_window); 
        Redactor_EndSDL(rs);
}

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


        SDL_RenderPresent(rs->sdl_renderer);
}

int Redactor_Main(int argc, char *argv[])
{
        Redactor rs = {0};
        Redactor_InitWin(&rs);
        Redactor_UseArgs(&rs, argc, argv);
        printf("%s", rs.file_data);
        while (rs.program_running) {
                Redactor_Cycle(&rs);
        }
        Redactor_End(&rs);
}
