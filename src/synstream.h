#include <SDL2/SDL.h>
#include "font.h"
#include "buffer.h"


/* draws buffer with syntax highlighting         
 * -------------------------------------         
 * returns: position plus size of last character */
SDL_Point draw_buffer(SDL_Renderer *renderer, struct font *font, struct buffer *buffer, SDL_Rect viewport)
{
    /* start rendering text from the viewport position */
    SDL_Point at = {viewport.x, viewport.y};

    for (int line = 0; line < buffer->line_count; ++line) {
        char *text = buffer->lines[line].data;
        SDL_Point size = font_write_text(font, text, at, renderer);

        /* move y by the line height, we are writing entire line, no X incerement needed */
        at.y += size.y;
    }

    return at;
}
