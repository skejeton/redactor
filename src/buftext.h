#include "dbg.h"
#include "font.h"
#include "rect.h"
#include "buffer.h"

struct buftext_pass {
    struct buffer *buffer;
    struct font *font;
    SDL_Renderer *renderer;
    SDL_Rect viewport;
    SDL_FPoint scroll;
    SDL_Color color; 
};

static void set_renderer_color_from_sdl_color(SDL_Renderer *renderer, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

static void begin_buffer_pass(struct buftext_pass *pass)
{
    SDL_RenderSetClipRect(pass->renderer, &pass->viewport);
}

static void end_buffer_pass(struct buftext_pass *pass)
{
    SDL_RenderSetClipRect(pass->renderer, NULL);
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

// Returns the rectangle size of the text that's right before the `marker`
// It considers the scroll and position
//------------------------------------------------------------------------
// x - The X position of the line
// y - The Y position of the line
// w - The width of the text that is before the marker
// h - The height of the line
static SDL_Rect get_buffer_text_pretext_rect(struct buftext_pass *pass, struct buffer_marker marker)
{
    marker = buffer_sanitize_marker(pass->buffer, marker);
    char *line = buffer_get_range(pass->buffer, buffer_marker_pretext_range(pass->buffer, marker));
    SDL_Point size = font_measure_text(pass->font, line);
    int y_offset = size.y * marker.line;
    SDL_Point position = get_buffer_text_pass_starting_position(pass);
    free(line);

    return (SDL_Rect) {
        .x = position.x,
        .y = position.y+y_offset,
        .w = size.x,
        .h = size.y
    };
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
