#include "docview.h"
#include "buftext.h"

static void draw_number_line(struct buftext_pass *pass)
{
    apply_buffer_text_pass(pass);
    SDL_Renderer *renderer = pass->renderer;
    struct font *font = pass->font;
    SDL_Point position = get_buffer_text_pass_starting_position(pass), line_size;
    const int right_padding = font_get_height(font);
    char number_buf[32];
    int max_width = 0;

    for (int i = 0; i < pass->buffer->line_count; ++i) {
        TODO("Replace snprintf with a faster int-to-string converter");
        snprintf(number_buf, sizeof number_buf, "%d", i+1);
        line_size = font_write_text(font, number_buf, position, renderer);
        position.y += line_size.y;
        if (max_width < line_size.x)
            max_width = line_size.x;
    }

    rect_cut_left(&pass->viewport, max_width + right_padding);
}

static SDL_Rect convert_pretext_rect_to_cursor(SDL_Rect rect)
{
    rect.x += rect.w;
    rect.w = 2;
    return rect;
}

static void draw_cursor(struct buftext_pass *pass, SDL_Color color, struct buffer_marker marker)
{
    SDL_Rect cursor_rect = convert_pretext_rect_to_cursor(get_buffer_text_pretext_rect(pass, marker));

    set_renderer_color_from_sdl_color(pass->renderer, color);
    SDL_RenderFillRect(pass->renderer, &cursor_rect);
}

void dv_draw(struct docview *view, SDL_Renderer *renderer)
{
    const SDL_Color primary_cursor_color = {220, 150, 0, 255};
    const SDL_Color secondary_cursor_color = {0, 150, 220, 255};
    struct buftext_pass text_pass = {
        .buffer = &view->document.buffer,
        .font = view->font,
        .renderer = renderer,
        .color = {250, 220, 190, 255},
        .viewport = view->viewport,
        .scroll = view->scroll
    };

    begin_buffer_pass(&text_pass);
    draw_number_line(&text_pass);
    draw_buffer_text(&text_pass);
    end_buffer_pass(&text_pass);
    draw_cursor(&text_pass, secondary_cursor_color, view->document.cursor.selection.from);
    draw_cursor(&text_pass, primary_cursor_color, view->document.cursor.selection.to);
}

void dv_scroll(struct docview *view, float dx, float dy)
{
    view->scroll.x += dx;
    view->scroll.y += dy;
}

void dv_tap(struct docview *view, bool shift, SDL_Point xy)
{

}