#ifndef BUFFERTAPE_H
#define BUFFERTAPE_H

#include <stdint.h>
#include "Buffer.h"
#include "Utf8.h"

struct {
    Buffer *buffer;
    Cursor cursor;
    uint32_t ch;
    Line line;
}
typedef BufferTape;

BufferTape BufferTape_Init(Buffer *buffer);
BufferTape BufferTape_InitAt(Buffer *buffer, Cursor cursor);

static inline void BufferTape_CacheChar(BufferTape *tape)
{
    if (*tape->line.text == 0 && tape->cursor.line < tape->buffer->lines_len-1) {
        tape->ch = '\n';
    } else {
        Utf8_Fetch(&tape->ch, tape->line.text);
    }
}

// Returns current character, advances the tape
static inline int BufferTape_Next(BufferTape *tape)
{
    uint32_t oldch = tape->ch;

    if (*tape->line.text == 0 && tape->cursor.line < tape->buffer->lines_len-1) {
        tape->line = tape->buffer->lines[tape->cursor.line+1];
    } else {
        tape->line.text += Utf8_Fetch(&tape->ch, tape->line.text);
    }

    switch (oldch) {
    case '\0': break;
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
    return tape->line.text-tape->buffer->lines[tape->cursor.line].text;
} 

#endif
