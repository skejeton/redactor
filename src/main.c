#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
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
bool is_file_new = false;
double cursort = 0;

const char *filename = "tests/main.c";
struct buffer buffer;

struct glyph {
    SDL_Texture *tex;
    SDL_Surface *surf;
    int h, w;
} glyphs[256];

void init_glyphs()
{
    for (int i = 0; i < 256; i += 1) {
        SDL_Surface *text_surface = TTF_RenderGlyph32_Blended(font, i, (SDL_Color) { 255, 255, 255, 255 });
        int h, w;
        if (text_surface != NULL) {
            h = text_surface->h;
            w = text_surface->w;
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            glyphs[i] = (struct glyph) {
                .h = h, .w = w,
                .tex = texture, .surf = text_surface
            };
        }
    }
}

static char* read_whole_file(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (f == NULL)
        return NULL;
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

SDL_Point write_text(int x, int y, const char *text)
{
    unsigned char r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    int rw = 0;
    if (!*text) { rw = 1; text = " "; };
    int w = 0, h = 0;
    for (;*text;text++) {
        SDL_SetTextureColorMod(glyphs[*text].tex, r, g, b);
        SDL_SetTextureAlphaMod(glyphs[*text].tex, a);
        h = h > glyphs[*text].h ? h : glyphs[*text].h;
        SDL_RenderCopy(renderer, glyphs[*text].tex, NULL, &(SDL_Rect){ x+w, y, glyphs[*text].w, glyphs[*text].h });
        if (!rw)
            w += glyphs[*text].w;
    
    }
    return (SDL_Point){w, h};
}

bool getnum(const char *text, const char ** end)
{
    const char *txt = text;
    while (*txt && isdigit(*txt))
        txt++;
    if (text != txt)
        *end = txt;
    return text != txt;
}

bool getstr(const char *text, const char **end)
{
    if (*text++ == '"') {
        while (*text && *text != '"') 
            if (*text++ == '\\')
                text++;
        text++;
        *end = text;
        return 1;
    }
    return 0;
}

bool getident(const char *text, const char **end)
{
    const char *txt = text;
    if (*txt && !(isalpha(*txt) || *txt == '_'))
        return false;
    while (*txt && (isalnum(*txt) || *txt == '_'))
        txt++;
    if (text != txt)
        *end = txt;
    return text != txt;

}

bool getcall(const char *text, const char **end)
{
    const char *prev = *end;
    if (getident(text, end) && **end == '(')
        return 1;
    *end = prev;
    return 0;
}

bool getspc(const char *text, const char **end) {
    const char *keytab[] = {
        "NULL", "EOF", "FILE",
    };

    for (int i = 0; i < sizeof(keytab)/sizeof(keytab[0]); i++) {
        int l = strlen(keytab[i]);
        if (strncmp(text, keytab[i], l) == 0 && !(isalnum(text[l]) || text[l] == '_')) {
            *end = text+l;
            return 1;
        }
    }

    return 0;
}

bool getkw(const char *text, const char **end) {
    const char *keytab[] = {
        "auto", "bool", "break", "case", "char",
        "const","continue","default","do",
        "double","else","enum","extern",
        "float","for","goto","if",
        "int","long","register","return",
        "short","signed","sizeof","static",
        "struct","switch","thread_local","typedef","union",
        "unsigned","void","volatile","while"
    };

    for (int i = 0; i < sizeof(keytab)/sizeof(keytab[0]); i++) {
        int l = strlen(keytab[i]);
        if (strncmp(text, keytab[i], l) == 0 && !(isalnum(text[l]) || text[l] == '_')) {
            *end = text+l;
            return 1;
        }
    }

    return 0;
}

bool getchr(const char *text, const char **end)
{
    if (*text++ == '\'') {
        while (*text && *text != '\'') 
            if (*text++ == '\\')
                text++;
        text++;
        *end = text;
        return 1;
    }
    return 0;
}

bool until_next_line(const char *text, const char **end)
{
    while (*text && *text++ != '\n')
        ;
    *end = text;
    return 1;
}

int write_line(int x, int y, const char *text) {
    int h = 0, w = 0;
    SDL_SetRenderDrawColor(renderer, 250, 220, 200, 16); 
    for (int i = 0;*text == ' ';i++,text++) {
        h = glyphs[' '].h;
        if (i % 4 == 0) {
            SDL_RenderFillRect(renderer, &(SDL_Rect){ x+w, y, 1, h });
        }
        w += glyphs[' '].w;
    }
    const char *end = text;
    for (;*text&&*end;text++) {
        const char *prevend = end;
        SDL_Color color;
        if (getnum(text, &end)) 
            color = (SDL_Color) {201, 134, 0, 255};
        else if (getkw(text, &end)) 
            color = (SDL_Color) {196, 97, 51, 255};
        else if (getspc(text, &end)) 
            color = (SDL_Color) {201, 85, 112, 255};
        else if (getcall(text, &end))
            color = (SDL_Color) {245, 90, 66, 255};
        else if (getident(text, &end)) 
            color = (SDL_Color) {250, 220, 200, 255};
        else if (getstr(text, &end)) 
            color = (SDL_Color) {95, 135, 35, 255};
        else if (getchr(text, &end)) 
            color = (SDL_Color) {95, 135, 35, 255};
        else if (strncmp(text, "//", 2) == 0 && until_next_line(text, &end))
            color = (SDL_Color) {82, 76, 60, 255};
        else if (strncmp(text, "#", 1) == 0 && until_next_line(text, &end))
            color = (SDL_Color) {196, 97, 51, 255};
        else
            continue;
        char tail[1024];

        SDL_SetRenderDrawColor(renderer, 250, 220, 200, 255);
        snprintf(tail, 1024, "%.*s", (int)(text-prevend), prevend);
        w += write_text(x+w, y, tail).x;

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        snprintf(tail, 1024, "%.*s", (int)(text-prevend), prevend);
        snprintf(tail, 1024, "%.*s", (int)(end-text), text);
        w += write_text(x+w, y, tail).x;
 

        text = end;

    }
    SDL_SetRenderDrawColor(renderer, 250, 220, 200, 255);
    h = write_text(x+w, y, end).y;
    return h;
}

bool ctrl = 0;
bool focused = 1;
int font_size = 18;

void load_font(int size) {
    if (font)
        TTF_CloseFont(font);
    font = TTF_OpenFont("data/monospace.ttf", size);
    if (font == NULL) {
        fprintf(stderr, "could not open font: %s\n", TTF_GetError());
        exit(1);
    }
    TTF_SetFontHinting(font, TTF_HINTING_LIGHT);
    init_glyphs();
}

void loop() {

    while (running) {
        cursort += 0.016;
        double prevt = cursort;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_TEXTINPUT: {
                    if (!ctrl) {
                        buffer_write(&buffer, event.text.text);
                        cursort = 0;
                    }
                } break;
                case SDL_MOUSEWHEEL:
                    if (event.wheel.y > 0)
                        buffer_move(&buffer, 0, -1);
                    if (event.wheel.y < 0)
                        buffer_move(&buffer, 0, 1);
                    break;
                case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        cursort = 0;
                        focused = 1;
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        focused = 0;
                        break;
                }
                case SDL_KEYDOWN:
                    cursort = 0;
                    switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_S:    
                        if (ctrl) {
                            buffer.dirty = 0;
                            char *file = buffer_get_whole_file(&buffer);
                            write_whole_file(filename, file);
                            free(file);
                        }
                        break;
                    case SDL_SCANCODE_MINUS:
                        if (ctrl && font_size > 10) load_font(--font_size);
                        break;
                    case SDL_SCANCODE_EQUALS:
                        if (ctrl && font_size < 64) load_font(++font_size);
                        break;
                    case SDL_SCANCODE_BACKSPACE:
                        buffer_erase_character(&buffer);
                        break;
                    case SDL_SCANCODE_UP:
                        buffer_move(&buffer, 0, -1);
                        break;
                    case SDL_SCANCODE_DOWN:
                        buffer_move(&buffer, 0, 1);
                        break;
                    case SDL_SCANCODE_LEFT:
                        buffer_move(&buffer, -1, 0);
                        break;
                    case SDL_SCANCODE_RIGHT:
                        buffer_move(&buffer, 1, 0);
                        break;
                    case SDL_SCANCODE_RETURN:
                        buffer_write(&buffer, "\n");
                        break;
                    case SDL_SCANCODE_TAB:
                        for (int i = 0, col = buffer.cursor.column; i < (4 - col % 4); i++)
                            buffer_write(&buffer, " ");
                        break;
                    case SDL_SCANCODE_LCTRL:
                    case SDL_SCANCODE_RCTRL:
                        ctrl = 1;
                        break;
                    default:
                        cursort = prevt;                   
                    }
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.scancode == SDL_SCANCODE_LCTRL ||
                        event.key.keysym.scancode == SDL_SCANCODE_RCTRL)
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
        SDL_RenderSetClipRect(renderer, &(SDL_Rect){0, 0, sw, sh-30}); 
        int h = 0;
        int glyphh = write_text(0, 0, "").y;
        for (int i = 0; i < buffer.line_count; ++i) {
            if ((10+h-buffer.cursor.line*glyphh+sh/2-15) > sh) {
                break;
            }
            char lineno[16];
            SDL_SetRenderDrawColor(renderer, 250, 220, 200, 32);
            if (buffer.cursor.line == i) {
                SDL_SetRenderDrawColor(renderer, 250, 220, 200, 128);
            }
            snprintf(lineno, 16, "%2d", i+1);
        
            h += write_text(10, 10+h-buffer.cursor.line*glyphh+sh/2-15, lineno).y;
        }
        h = 0;
        for (int i = 0; i < buffer.line_count; ++i) {
             h += write_line(50, 10+h-buffer.cursor.line*glyphh+sh/2-15, buffer.lines[i].data);
        }
        SDL_RenderSetClipRect(renderer, NULL); 
        char *l = buffer_get_trimmed_line_at(&buffer, buffer.cursor.column);
        // HACK: Fixing the position of the cursor
        SDL_Point wh = write_text(-10000, -10000, l);
        free(l);
        
        SDL_SetRenderDrawColor(renderer, 220, 150, 0, 255.0*(cos(cursort*8)/2+0.5));
        if (focused)
            SDL_RenderFillRect(renderer, &(SDL_Rect){ wh.x+50, 10+sh/2-15, 2, wh.y });
        
        SDL_SetRenderDrawColor(renderer, 250, 220, 200, 10);
        /*
        SDL_RenderFillRect(renderer, &(SDL_Rect){40, 0, 1, sh - 30});*/
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, sh - 30, 2000, 1});
        SDL_SetRenderDrawColor(renderer, 250, 220, 200, 128);
        char txt[1024];
        snprintf(txt, 1024, "%s %s%s%d:%d fs: %d", filename, is_file_new ? "(new) " : "", buffer.dirty ? "* " : "", buffer.cursor.line+1, buffer.cursor.column+1, font_size);
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
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); 
    if (renderer == NULL) {
        fprintf(stderr, "couldn't create renderer: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    load_font(font_size);

    init_glyphs();
    buffer = buffer_init();
    char *file = read_whole_file(filename);
    is_file_new = file == NULL;
    if (file != NULL) {
        // NOTE: Quick and dirty fix for extraneous line appended!
        if (*file && file[strlen(file)-1] == '\n')
            file[strlen(file)-1] = 0;
   
        buffer_write(&buffer, file);
    }
    

    buffer.cursor.column = 0;
    buffer.cursor.line = 0;
    buffer.dirty = 0;
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
