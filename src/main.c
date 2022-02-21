#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_hints.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "buffer.h"

SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font;
bool running = true;

const char *filename = "tests/main.c";
struct buffer buffer;

static char* read_whole_file(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (f == NULL)
        return strdup("");
    fseek(f, 0, SEEK_END);
    size_t fsz = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    char* input = (char*)malloc((fsz + 1) * sizeof(char));
    input[fsz] = 0;
    fread(input, sizeof(char), fsz, f);
    fclose(f);
    return input;
}

static void write_whole_file(const char* path, const char *contents)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL)
        return;

    fwrite(contents, strlen(contents), 1, f);
    fclose(f);
}

int write_text(int x, int y, const char *text)
{
    unsigned char r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    if (!*text) text = " ";
    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font, text, (SDL_Color) { r, g, b, a });
    int h = 0;
    if (text_surface != NULL) {
        h = text_surface->h;
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        SDL_RenderCopy(renderer, texture, NULL, &(SDL_Rect){ x, y, text_surface->w, text_surface->h});
        SDL_FreeSurface(text_surface);
        SDL_DestroyTexture(texture);
    }
    return h;
}
bool ctrl = 0;


void loop() {


    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_TEXTINPUT: {
                    buffer_write(&buffer, event.text.text);
                } break;
                case SDL_KEYDOWN:
                    if (ctrl && event.key.keysym.scancode == SDL_SCANCODE_S) {
                        buffer.dirty = 0;
                        char *file = buffer_get_whole_file(&buffer);
                        write_whole_file(filename, file);
                        free(file);
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) 
                        buffer_erase_character(&buffer);
                    if (event.key.keysym.scancode == SDL_SCANCODE_UP)
                        buffer_move(&buffer, 0, -1);
                    if (event.key.keysym.scancode == SDL_SCANCODE_DOWN)
                        buffer_move(&buffer, 0, 1);
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT)
                        buffer_move(&buffer, -1, 0);
                    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT)
                        buffer_move(&buffer, 1, 0);
                    if (event.key.keysym.scancode == SDL_SCANCODE_RETURN)
                        buffer_write(&buffer, "\n");
                    if (event.key.keysym.scancode == SDL_SCANCODE_TAB)
                        buffer_write(&buffer, "    ");
                    if (event.key.keysym.scancode == SDL_SCANCODE_LCTRL)
                        ctrl = 1;
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.scancode == SDL_SCANCODE_LCTRL)
                        ctrl = 0;
                    break;
                case SDL_QUIT:
                    running = 0;
            }   
        }

        SDL_SetRenderDrawColor(renderer, 32, 26, 23, 255);
        SDL_RenderClear(renderer);
        int sw, sh;
        SDL_GetWindowSize(window, &sw, &sh);

/*
        SDL_SetRenderDrawColor(renderer, 11, 8, 8, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, 40, sh - 30});
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, sh - 30, 2000, 30});
*/
        int h = 0;
        int glyphh = write_text(0, 0, "");
        for (int i = 0; i < buffer.line_count; ++i) {
            char lineno[16];
            SDL_SetRenderDrawColor(renderer, 250, 220, 200, 32);
            if (buffer.cursor.line == i) {
                SDL_SetRenderDrawColor(renderer, 250, 220, 200, 128);
            }
            snprintf(lineno, 16, "%2d", i+1);
            h += write_text(10, 10+h-buffer.cursor.line*glyphh+sh/2-15, lineno);
        }
        SDL_SetRenderDrawColor(renderer, 250, 220, 200, 255);
        h = 0;
        for (int i = 0; i < buffer.line_count; ++i) {
             h += write_text(50, 10+h-buffer.cursor.line*glyphh+sh/2-15, buffer.lines[i].data);
        }
        int w;
        char *l;
        TTF_SizeUTF8(font, l = buffer_get_trimmed_line_at(&buffer, buffer.cursor.column), &w, &h);
        free(l);
        SDL_SetRenderDrawColor(renderer, 220, 150, 0, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){ w+50, 10+sh/2-15, 2, h });
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        SDL_SetRenderDrawColor(renderer, 250, 220, 200, 10);
        SDL_RenderFillRect(renderer, &(SDL_Rect){40, 0, 1, sh - 30});
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, sh - 30, 2000, 1});
        SDL_SetRenderDrawColor(renderer, 250, 220, 200, 128);
        char txt[1024];
        snprintf(txt, 1024, "%s %s %d:%d", filename, buffer.dirty ? "*" : "", buffer.cursor.line+1, buffer.cursor.column+1);
        write_text(0+5, sh-30+5, txt);
        SDL_RenderPresent(renderer);
    }
}

int main(int argc, char *argv[]) {
    if (argc == 2)
        filename = argv[1];
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        fprintf(stderr, "could not initialize sdl ttf: %s\n", TTF_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
            "redactor",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1920/2, 1080/2,
            SDL_WINDOW_SHOWN
            );
    if (window == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, 0, 0); 
    if (renderer == NULL) {
        fprintf(stderr, "couldn't create renderer: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    font = TTF_OpenFont("data/monospace.ttf", 17);
    if (font == NULL) {
        fprintf(stderr, "could not open font: %s\n", TTF_GetError());
        return 1;
    }

    char *file = read_whole_file(filename);
    buffer = buffer_init();
    buffer_write(&buffer, file);
    buffer.cursor.column = 0;
    buffer.cursor.line = 0;
    buffer_move(&buffer, 0, 0);
    free(file);
    loop();

    buffer_deinit(&buffer);
    buffer = (struct buffer) { 0 };

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

