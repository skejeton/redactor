#include "docview.h"
#include "font.h"
#include "rect.h"
#include "utf8.h"
#include "dbg.h"
#include <limits.h>

static struct buffer_range line_slice_from_range(struct buffer *buffer, struct buffer_range *range)
{
    struct buffer_range slice = {0};
    slice.from.line = range->from.line;
    slice.to.line = range->from.line;
    slice.from.column = range->from.column;

    if (range->from.line != range->to.line) {
        slice.to.column = buffer_line_length(buffer, range->from.line);
        range->from.line += 1;
        range->from.column = 0;
    } else {
        slice.to.column = range->to.column;
        range->from.column = range->to.column;
    }
    return slice;
}

static SDL_Point map_point_from_marker_to_screen(struct font *font, struct buffer *buffer, struct buffer_marker marker)
{
    marker = buffer_sanitize_marker(buffer, marker);
    int font_height = font_get_height(font);
    struct buffer_line *line = &buffer->lines[marker.line];
    char *data = buffer_get_range(buffer, buffer_marker_pretext_range(buffer, marker));
    SDL_Point position = {font_measure_text(font, data).x, font_height * marker.line};
    free(data);
    return position;
}

struct buffer_marker map_point_from_screen_to_marker(struct font *font, struct buffer *buffer, SDL_Point point)
{
    int font_height = font_get_height(font);
    int minimum_distance = INT_MAX;
    int width = 0;
    struct buffer_marker marker = {point.y/font_height, 0};
    if (marker.line < 0)
        marker.line = 0;
    if (marker.line > buffer->line_count-1)
        marker.line = buffer->line_count-1;

    int line_size = buffer->lines[marker.line].size;
    const char *line = buffer->lines[marker.line].data;
    int line_length = buffer_line_length(buffer, marker.line);

    while (marker.column <= line_length) {
        int c = utf8_get(&line, &line_size);
        int distance = abs(point.x - width);
        if (minimum_distance > distance) {
            marker.column++;
            minimum_distance = distance;
        } else {
            marker.column--;
            break;
        }
        width += font_measure_glyph(font, c).x;
    }
    return marker;
}

static SDL_Rect make_line_slice_rectangle(struct font *font, struct buffer *buffer, struct buffer_range slice)
{
    int font_height = font_get_height(font);
    SDL_Point position = map_point_from_marker_to_screen(font, buffer, slice.from);
    SDL_Point end_position = map_point_from_marker_to_screen(font, buffer, slice.to);
    return (SDL_Rect) {position.x, position.y, end_position.x-position.x, font_height};
}

static void draw_buffer_lines(struct buffer *buffer, SDL_Point position, SDL_Renderer *renderer, struct font *font)
{
    SDL_SetRenderDrawColor(renderer, 255, 240, 230, 255);
    for (int i = 0; i < buffer->line_count; ++i) {
        const char *line = buffer->lines[i].data;
        position.y += font_write_text(font, line, position, renderer).y;
    }
}


static SDL_Rect get_marker_rect(struct font *font, struct buffer *buffer, struct buffer_marker marker)
{
    SDL_Rect pretext_rect = make_line_slice_rectangle(font, buffer, buffer_marker_pretext_range(buffer, marker));
    pretext_rect.x = pretext_rect.w;
    pretext_rect.w = 0;
    return pretext_rect;
}

static void draw_marker(SDL_Renderer *renderer, struct font *font, SDL_Point offset, struct buffer *buffer, struct buffer_marker marker)
{
    SDL_Rect cursor_rect = rect_offset(get_marker_rect(font, buffer, marker), offset);
    SDL_RenderDrawRect(renderer, &cursor_rect);
}

struct cursor_colorscheme {
    SDL_Color from;
    SDL_Color to;
};

static void set_draw_color_from_color(SDL_Renderer *renderer, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

static void draw_range_cursors(SDL_Renderer *renderer, struct cursor_colorscheme colorscheme, struct buffer *buffer, struct font *font, SDL_Point offset, struct buffer_range range)
{
    set_draw_color_from_color(renderer, colorscheme.from);
    draw_marker(renderer, font, offset, buffer, range.from);
    set_draw_color_from_color(renderer, colorscheme.to);
    draw_marker(renderer, font, offset, buffer, range.to);
}

static struct cursor_colorscheme default_cursor_colorscheme()
{
    return (struct cursor_colorscheme) {
        {0, 150, 220, 255}, {220, 150, 0, 255}
    };
}

SDL_Rect make_line_slice_hightlight_rect(struct font *font, struct buffer *buffer, struct buffer_range *range)
{
    bool newline_at_end = range->from.line != range->to.line;
    struct buffer_range line_slice = line_slice_from_range(buffer, range);
    SDL_Rect slice_rectangle = make_line_slice_rectangle(font, buffer, line_slice);
    if (newline_at_end)
        slice_rectangle.w += font_measure_glyph(font, ' ').x;
    return slice_rectangle;
}

static void draw_highlight_area(SDL_Renderer *renderer, SDL_Point offset, struct font *font, struct buffer *buffer, struct buffer_range range)
{
    SDL_SetRenderDrawColor(renderer, 255, 240, 230, 16);
    while (!buffer_range_empty(range)) {
        SDL_Rect slice_rectangle = rect_offset(make_line_slice_hightlight_rect(font, buffer, &range), offset);
        SDL_RenderFillRect(renderer, &slice_rectangle);
    } 
}

static void draw_space_highlight_marker(SDL_Renderer *renderer, SDL_Rect rect)
{
    rect = rect_center(rect, (SDL_Rect){0, 0, rect.w/4, rect.h/8});
    SDL_RenderFillRect(renderer, &rect);
}

static void draw_highlight_particles(SDL_Renderer *renderer, SDL_Point offset, struct font *font, struct buffer *buffer, struct buffer_range range)
{
    char *orig = buffer_get_range(buffer, range);   
    const char *text = orig;
    TODO("This is inefficent");
    int max = strlen(text), c;
    int font_height = font_get_height(font);
    SDL_Point position = point_add(map_point_from_marker_to_screen(font, buffer, range.from), offset);

    while ((c = utf8_get(&text, &max))) {
        SDL_Point glyph_size = font_measure_glyph(font, c);
        switch (c) {
            case ' ': case '\t':
                draw_space_highlight_marker(renderer, (SDL_Rect){position.x, position.y, glyph_size.x, glyph_size.y});
            break;
        }

        if (c == '\n') {
            position.x = offset.x;
            position.y += font_height;
        } else {
            position.x += glyph_size.x;
        }
    }
    free(orig);
}

static void draw_highlight(SDL_Renderer *renderer, SDL_Point offset, struct font *font, struct buffer *buffer, struct buffer_range range)
{
    range = buffer_sanitize_range(buffer, range);
    draw_highlight_area(renderer, offset, font, buffer, range);
    SDL_SetRenderDrawColor(renderer, 255, 240, 230, 16);
    draw_highlight_particles(renderer, offset, font, buffer, range);
}

int draw_number_column(SDL_Rect *rect, SDL_Renderer *renderer, struct font *font, int line_count)
{
    int max_width = 0;
    SDL_Point position = {0, 0};

    for (int i = 1; i <= line_count; ++i) {
        char s[32];
        snprintf(s, 32, "%d", i);

        SDL_Point size = font_write_text(font, s, point_add(position, rect_position(*rect)), renderer);
        if (size.x > max_width)
            max_width = size.x;
        position.y += size.y;
    }

    max_width += font_get_size(font);

    rect_cut_left(rect, max_width);
    return max_width;
}

SDL_FPoint fix_scroll(SDL_FPoint scroll, SDL_Point max_bounds)
{
    if (scroll.x < 0)
        scroll.x = 0;
    if (scroll.y < 0)
        scroll.y = 0;
    if (scroll.y > max_bounds.y)
        scroll.y = max_bounds.y;
    return scroll;
}

static SDL_Point focus_delta(SDL_Rect bounds, SDL_Rect rect)
{
    SDL_Rect new_rect = rect_keep_in_bounds(bounds, rect);
    return point_sub(rect_position(rect), rect_position(new_rect));
}

static void focus_cursor_on_viewport(SDL_Rect viewport, struct docview *view)
{
    SDL_Rect marker_rect = rect_offset(get_marker_rect(view->font, &view->document.buffer, view->document.cursor.selection.to), rect_position(viewport));
    SDL_Point delta = focus_delta(rect_inset(view->viewport, 5), marker_rect);
    printf("%d %d\n", delta.x, delta.y);
    view->viewport_scroll.x += delta.x;
    view->viewport_scroll.y += delta.y;
}

void dv_draw(struct docview *view, SDL_Renderer *renderer)
{
    SDL_Point maxima = map_point_from_marker_to_screen(view->font, &view->document.buffer, (struct buffer_marker){10000, 10000});
    view->viewport_scroll = fix_scroll(view->viewport_scroll, maxima);
    SDL_RenderSetClipRect(renderer, &view->viewport);
    SDL_Rect viewport = rect_offset(view->viewport, (SDL_Point){-view->viewport_scroll.x, -view->viewport_scroll.y});
    SDL_SetRenderDrawColor(renderer, 255, 240, 230, 32);
    view->number_line_width = draw_number_column(&viewport, renderer, view->font, view->document.buffer.line_count);
    SDL_Point position = rect_position(viewport);
    focus_cursor_on_viewport(viewport, view);
    draw_buffer_lines(&view->document.buffer, position, renderer, view->font);
    draw_range_cursors(renderer, default_cursor_colorscheme(), &view->document.buffer, view->font, position, view->document.cursor.selection);
    draw_highlight(renderer, position, view->font, &view->document.buffer, view->document.cursor.selection);
    SDL_RenderSetClipRect(renderer, NULL);
}

void dv_scroll(struct docview *view, float dx, float dy)
{
    view->viewport_scroll.x -= dx;
    view->viewport_scroll.y -= dy;
}

void dv_tap(struct docview *view, bool shift, SDL_Point xy)
{
    SDL_Rect viewport = rect_offset(view->viewport, (SDL_Point){view->viewport_scroll.x+view->number_line_width, view->viewport_scroll.y});
    xy = point_sub(xy, rect_position(viewport));
    struct buffer_marker marker = map_point_from_screen_to_marker(view->font, &view->document.buffer, xy);
    de_set_cursor(&view->document, shift, marker);
}