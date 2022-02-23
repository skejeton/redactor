#include <SDL2/SDL_render.h>
#include "buffer.h"
#include "hl.c"
#include "docview.h"


static void draw_line_number(int l, SDL_Point position, SDL_Renderer *renderer, struct docview *view)
{
    SDL_SetRenderDrawColor(renderer, 250, 220, 200, 32);
    if (view->buffer.cursor.line == l) {
        SDL_SetRenderDrawColor(renderer, 250, 220, 200, 128);
    }

    char buf[16];
    snprintf(buf, 16, "%2d", l+1);
    font_write_text(buf, (SDL_Point){position.x, position.y}, renderer, view->font);
}


static SDL_Point draw_lines(SDL_Rect viewport, SDL_Renderer *renderer, struct docview *view)
{
    struct buffer *buffer = &view->buffer;
    SDL_Point position = (SDL_Point) { viewport.x, viewport.y };
    
    for (int i = 0; i < buffer->line_count; i++) {
        // NOTE: I might want to take average of the delta produced by write_line and 
        draw_line_number(i, position, renderer, view); 

        // TODO: Handle the offset more appropriately
        SDL_Point line_position = (SDL_Point) { position.x + 40, position.y };
        position.y += write_line(buffer->lines[i].data, line_position, view->font, renderer);
    }
    return position;
}


static SDL_Rect get_cursor_rect(SDL_Rect viewport, struct docview *view)
{
    SDL_Point viewport_pos = {viewport.x+40, viewport.y};
    char *line = buffer_get_trimmed_line_at(&view->buffer, view->buffer.cursor.column);
    SDL_Point line_size = font_measure_text(line, view->font);
    free(line);
    int at_line = view->buffer.cursor.line;
    SDL_Point cursor_position = { viewport_pos.x + line_size.x, viewport_pos.y + line_size.y * at_line };
    SDL_Rect cursor_rect = { cursor_position.x, cursor_position.y, 2, line_size.y };
    return cursor_rect;
}


static void draw_cursor(SDL_Rect viewport, SDL_Renderer *renderer, struct docview *view)
{
    SDL_Rect cursor_rect = get_cursor_rect(viewport, view);
    SDL_SetRenderDrawColor(renderer, 220, 150, 0, 255.0*(cos(view->blink*8)/2+0.5));
    SDL_RenderFillRect(renderer, &cursor_rect);
}


static void focus_on_cursor(SDL_Rect viewport, struct docview *view)
{
    SDL_Rect cursor_rect = get_cursor_rect(viewport, view);
    cursor_rect.y -= viewport.y;
    if (cursor_rect.y > (view->scroll.y+viewport.h-cursor_rect.h))
        view->scroll.y = cursor_rect.y+cursor_rect.h-viewport.h;
    if (cursor_rect.y < viewport.y+view->scroll.y)
        view->scroll.y = cursor_rect.y;
}

void docview_tap(SDL_Rect viewport, SDL_Point xy, struct docview *view)
{
    SDL_Point screen = {
        viewport.x+xy.x+view->scroll_damped.x-40,
        viewport.y+xy.y+view->scroll_damped.y,
    };
    
    // FIXME: This assumes a monospace font!!!!!
    SDL_Point glyph_size = font_measure_glyph(' ', view->font);
    
    view->buffer.cursor.column = 0;
    view->buffer.cursor.line = 0;
    buffer_move(&view->buffer, screen.x/glyph_size.x-2, screen.y/glyph_size.y-1);
}

void docview_draw(SDL_Rect viewport, SDL_Renderer *renderer, struct docview *view)
{
    // TODO: Use deltatime
    view->blink += 0.016;
    // Reset cursor blink after a movement
    if (view->prev_cursor_pos.line != view->buffer.cursor.line ||
        view->prev_cursor_pos.column != view->buffer.cursor.column) {
        view->blink = 0;
        focus_on_cursor(viewport, view);
    }
        
    view->prev_cursor_pos = view->buffer.cursor;
    
    SDL_RenderSetClipRect(renderer, &viewport);

    if (view->scroll.y < 0) 
        view->scroll.y = 0;
    // Smooth out the scrolling
    view->scroll_damped.x += (view->scroll.x-view->scroll_damped.x)/5;
    view->scroll_damped.y += (view->scroll.y-view->scroll_damped.y)/5;
    
    viewport.y -= view->scroll_damped.y;
    SDL_Point buffer_size = draw_lines(viewport, renderer, view);    
    
    // Clamp scrolling to not scroll outside bounds
    if (view->scroll.y > buffer_size.y-viewport.y-50)
        view->scroll.y = buffer_size.y-viewport.y-50;
    
    // HACK: adding hardcoded offset to the cursor position to align it with the line values
    draw_cursor(viewport, renderer, view);
    
    SDL_RenderSetClipRect(renderer, NULL);
}
