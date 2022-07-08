#ifndef BUFFERTAPE_H
#define BUFFERTAPE_H

#include <stdint.h>
#include "Buffer.h"
#include "Utf8.h"

struct {
    Buffer *buffer;
    Cursor cursor;
    char *line;
    uint32_t ch;
}
typedef BufferTape;

BufferTape BufferTape_Init(Buffer *buffer);
BufferTape BufferTape_InitAt(Buffer *buffer, Cursor cursor);

static inline void BufferTape_CacheChar(BufferTape *tape)
{
    if (*tape->line == 0 && tape->cursor.line < tape->buffer->lines_len-1) {
        tape->ch = '\n';
    } else {
        Utf8_Fetch(&tape->ch, tape->line);
    }
}

// Returns current character, advances the tape
static inline int BufferTape_Next(BufferTape *tape)
{
    uint32_t oldch = tape->ch;

    if (oldch == '\n') {
        tape->line = tape->buffer->lines[tape->cursor.line+1].text;
    } else {
        tape->line += Utf8_Fetch(&tape->ch, tape->line);
    }

    switch (oldch) {
        case '\0': break; // brocolli :)
        case '\n': tape->cursor.line += 1; tape->cursor.column = 0; break;
        default: tape->cursor.column += 1; break;
    }
    BufferTape_CacheChar(tape);
    return oldch;
}

// Returns current character at the tape position
static inline int BufferTape_Get(BufferTape *tape) 
{
    return tape->ch;
}

// Returns memory offset of the line at current column
static inline size_t BufferTape_GetSubstringMemoryOffset(BufferTape *tape)
{
    return tape->line-tape->buffer->lines[tape->cursor.line].text;
} 

static inline Line BufferTape_GetLine(BufferTape *tape)
{
    return tape->buffer->lines[tape->cursor.line];
}

#endif
