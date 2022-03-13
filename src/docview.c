#include "docview.h"
#include "buftext.h"

static void draw_number_line(struct buftext_pass *pass)
{
    apply_buffer_text_pass(pass);
    struct buffer *buffer = pass->buffer;
    SDL_Renderer *renderer = pass->renderer;
    struct font *font = pass->font;
    SDL_Point position = get_buffer_text_pass_starting_position(pass), line_size;
    const int number_line_padding = font_get_height(font);
    char line_number_text_buffer[32];
    int maximum_line_width = 0;

    for (int i = 0; i < buffer->line_count; ++i) {
        TODO("Replace snprintf with a faster int-to-string converter");
        snprintf(line_number_text_buffer, sizeof line_number_text_buffer, "%d", i+1);
        line_size = font_write_text(font, line_number_text_buffer, position, renderer);
        position.y += line_size.y;
        if (maximum_line_width < line_size.x)
            maximum_line_width = line_size.x;
    }

    rect_cut_left(&pass->viewport, maximum_line_width + number_line_padding);
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

    draw_number_line(&text_pass);
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