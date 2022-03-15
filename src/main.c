#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "buffer.h"
#include "docview.h"
#include "util.h"
#include "input.h"
#include "dbg.h"

SDL_Window *window;
SDL_Renderer *renderer;
struct docview document;
const char *font_path = "data/monospace.ttf";
struct font *font;
bool running = true;
bool is_file_new = false;
const char *filename = "tests/specimen/main.c";
struct input_state input_state;
int screen_width, screen_height;

static struct input_pass get_input_pass(SDL_Event *event)
{
    return (struct input_pass) {
        .event = event, .filename = filename, .running = &running,
        .view = &document, .window = window, .renderer = renderer,
    };
}

void draw_statusbar(struct view *view, struct docedit *editor)
{
    SDL_Rect view_rect = ui_get_view_rect(view);
    char txt[1024];
    SDL_SetRenderDrawColor(renderer, 250, 220, 200, 128);
    snprintf(txt, 1024, "%s %s%s%d:%d fs: %d", 
        filename, is_file_new ? "(new) " : "", editor->buffer.dirty ? "* " : "",
        editor->cursor.selection.to.line+1, editor->cursor.selection.to.column+1, font_get_size(font));

    SDL_Point text_size = font_measure_text(font, txt);
    SDL_Rect text_rect = { view_rect.x, view_rect.y, text_size.x, text_size.y };
    text_rect = ui_center_y(view, text_rect);
    SDL_Point text_position = { text_rect.x, text_rect.y };

    font_write_text(font, txt, text_position, renderer);
}

void draw_view(struct view *view)
{
    SDL_Rect view_rect = ui_get_view_rect(view);
    SDL_SetRenderDrawColor(renderer, 32, 26, 23, 255);
    SDL_RenderFillRect(renderer, &view_rect);
}

void ui()
{
    struct view screen, docview, statusbar;
    screen = ui_default_view(0, 0, screen_width, screen_height);
    ui_inset(&screen, font_get_size(document.font));
    statusbar = ui_cut_bottom(&screen, font_measure_glyph(font, ' ').y);
    docview = screen;
    document.viewport = ui_get_view_rect(&docview);

    draw_view(&docview);
    draw_view(&statusbar);

    dv_draw(&document, renderer);
    draw_statusbar(&statusbar, &document.document);
}

void loop() {

    while (running) {
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        SDL_GetWindowSize(window, &screen_width, &screen_height);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            input_process_event(&input_state, get_input_pass(&event));
        }

        
        if (input_state.leftmousedown && input_state.focused) {
            dv_tap(&document, true, (SDL_Point){mouse_x, mouse_y});
        }
        SDL_SetRenderDrawColor(renderer, 32, 26, 23, 255);
        SDL_RenderClear(renderer);
        ui();
        SDL_RenderPresent(renderer);
    }
}

static struct buffer create_buffer_from_string(const char *string)
{
    struct buffer buf = buffer_init();
    if (string != NULL)
        buffer_insert(&buf, (struct buffer_marker){0, 0}, string);
    // since the file is just opened it's not actually dirty
    buf.dirty = 0;
    return buf;
}

// returns true if file is new
static bool init_document_from_file(const char *filename, struct docedit *editor)
{
    char *file = util_read_whole_file(filename);
    editor->buffer = create_buffer_from_string(file);
    free(file);
    return file == NULL;
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
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
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
    SDL_Cursor *cur = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    SDL_SetCursor(cur);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    font = document.font = font_init(font_path, 17, renderer);

    is_file_new = init_document_from_file(filename, &document.document);
    
    loop();
    SDL_FreeCursor(cur);
    // TODO: Handle this in docview instead
    buffer_deinit(&document.document.buffer);
    document.document.buffer = (struct buffer) { 0 };


    font_deinit(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
