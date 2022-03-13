#include "dbg.h"
#include "font.h"
#include "rect.h"
#include "buffer.h"
#include "utf8.h"

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
    return (SDL_Point){position.x + pass->scroll.x, position.y - pass->scroll.y};
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

static int calculate_best_column_from_mouse(struct font *font, const char *line, int size, int x)
{
    int32_t width = 0, column = 0, distance, minimum_distance = INT32_MAX;

    for (int ch = 0; (ch = utf8_get(&line, &size)); ++column) {
        distance = abs(x - width);
        if (minimum_distance > distance) 
            minimum_distance = distance;
        width += font_measure_glyph(font, ch).x;
        if (x - width < 0)
            return column;
    }
    return column;
}

static struct buffer_marker map_position_to_marker(struct buftext_pass *pass, SDL_Point point)
{
    point = point_sub(point, get_buffer_text_pass_starting_position(pass));
    int font_height = font_get_height(pass->font);
    int line_number = point.y / font_height;
    int line_total = pass->buffer->line_count;
    if (line_number < 0)
        line_number = 0;
    else if (line_number > line_total-1)
        line_number = line_total-1;

    char *line_text = pass->buffer->lines[line_number].data;
    int line_size = pass->buffer->lines[line_number].size;
    int column_number = calculate_best_column_from_mouse(pass->font, line_text, line_size, point.x);
    return (struct buffer_marker){line_number, column_number};
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
