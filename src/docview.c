#include "docview.h"
#include "dbg.h"
#include "font.h"
#include "rect.h"

struct buftext_pass {
    struct buffer *buffer;
    struct font *font;
    SDL_Renderer *renderer;
    SDL_Rect viewport;
    SDL_FPoint scroll;
    SDL_Color color; 
};

struct buftext_style {
};

static void set_renderer_color_from_sdl_color(SDL_Renderer *renderer, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

static void apply_buffer_text_pass(struct buftext_pass *pass)
{
    set_renderer_color_from_sdl_color(pass->renderer, pass->color);
}

static SDL_Point get_buffer_text_pass_starting_position(struct buftext_pass *pass)
{
    SDL_Point position = rect_position(pass->viewport);
    return (SDL_Point){position.x + pass->scroll.x, position.y + pass->scroll.y};
}

static void draw_buffer_text(struct buftext_pass *pass)
{
    SDL_Point line_size, position = get_buffer_text_pass_starting_position(pass);
    char *line;
    apply_buffer_text_pass(pass);

    for (int i = 0; i < pass->buffer->line_count; ++i) {
        line = pass->buffer->lines[i].data;
        line_size = font_write_text(pass->font, line, position, pass->renderer);
        position.y += line_size.y;
    }
}

void dv_draw(struct docview *view, SDL_Renderer *renderer)
{
    struct buftext_pass text_pass = {
        .buffer = &view->document.buffer,
        .font = view->font,
        .renderer = renderer,
        .color = {250, 220, 190, 255},
        .viewport = view->viewport,
        .scroll = view->scroll
    };

    draw_buffer_text(&text_pass);
}

void dv_scroll(struct docview *view, float dx, float dy)
{
    view->scroll.x += dx;
    view->scroll.y += dy;
}

void dv_tap(struct docview *view, bool shift, SDL_Point xy)
{

}