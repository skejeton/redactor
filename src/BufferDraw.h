#ifndef R_BUFFERDRAW_H
#define R_BUFFERDRAW_H
#include <SDL2/SDL.h>
#include "Buffer.h"

struct Redactor;

struct {
    size_t line;    // Line index
    size_t offset;  // Line string offset (NOT column)
    int column;     // Column at which segment is at
    SDL_Color color;// RGBA color
}
typedef BufferDrawSegment;

struct BufferDrawSegments {
    BufferDrawSegment *segments;
    size_t segments_len;
}
typedef BufferDrawSegments;

// Removes all segments from the segment list, invalidating them
void BufferDraw_InvalidateSegments(BufferDrawSegments *seg);
// Adds a segment relative to previous segment
void BufferDraw_InsertSegment(BufferDrawSegments *seg, size_t line, int column, size_t offset, SDL_Color color);
// Draws segmented buffer
void BufferDraw_DrawBuffer(struct Redactor *rs, Buffer *buf, BufferDrawSegments *seg);

#endif
