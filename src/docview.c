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


static void draw_lines(SDL_Rect viewport, SDL_Renderer *renderer, struct docview *view)
{
    struct buffer *buffer = &view->buffer;
    SDL_Point position = (SDL_Point) { viewport.x, viewport.y };
    
    for (int i = 0; i < buffer->line_count; i++) {
        // NOTE: I might want to take averate of the delta produced by write_line and 
        draw_line_number(i, position, renderer, view); 

        // TODO: Handle the offset more appropriately
        SDL_Point line_position = (SDL_Point) { position.x + 40, position.y };
        position.y += write_line(buffer->lines[i].data, line_position, view->font, renderer);
    }
}


static void draw_cursor(SDL_Rect viewport, SDL_Renderer *renderer, struct docview *view)
{
    SDL_Point viewport_pos = {viewport.x+40, viewport.y};
    char *line = buffer_get_trimmed_line_at(&view->buffer, view->buffer.cursor.column);
    SDL_Point line_size = font_measure_text(line, view->font);
    free(line);
    int at_line = view->buffer.cursor.line;
    SDL_Point cursor_position = { viewport_pos.x + line_size.x, viewport_pos.y + line_size.y * at_line };
    SDL_Rect cursor_rect = { cursor_position.x, cursor_position.y, 2, line_size.y };

    if (cursor_position.y > (viewport.h-cursor_rect.h))
        view->scroll.y -= viewport.h-(cursor_position.y+cursor_rect.h)+(viewport.y+view->scroll.y);
    if (cursor_position.y < viewport.y+view->scroll.y)
        view->scroll.y += cursor_position.y-(viewport.y+view->scroll.y);

    SDL_SetRenderDrawColor(renderer, 220, 150, 0, 255.0*(cos(view->blink*8)/2+0.5));
    SDL_RenderFillRect(renderer, &cursor_rect);
}


void docview_draw(SDL_Rect viewport, SDL_Renderer *renderer, struct docview *view)
{
    // TODO: Use deltatime
    view->blink += 0.016;
    // Reset cursor blink after a movement
    if (view->prev_cursor_pos.line != view->buffer.cursor.line ||
        view->prev_cursor_pos.column != view->buffer.cursor.column) {
        view->blink = 0;
    }
    
    view->prev_cursor_pos = view->buffer.cursor;
    
    SDL_RenderSetClipRect(renderer, &viewport);

    viewport.y -= view->scroll.y;
    draw_lines(viewport, renderer, view);
    
    // HACK: adding hardcoded offset to the cursor position to align it with the line values
    draw_cursor(viewport, renderer, view);
    
    SDL_RenderSetClipRect(renderer, NULL);
}
