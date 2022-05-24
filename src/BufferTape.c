#include <stdint.h>
#include <assert.h>
#include "BufferTape.h"
#include "Utf8.h"

static void In_CacheChar(BufferTape *tape)
{
    if (*tape->line.text == 0 && tape->cursor.line < tape->buffer->lines_len-1) {
        tape->ch = '\n';
    } else {
        Utf8_Fetch(&tape->ch, tape->line.text);
    }
}

BufferTape BufferTape_Init(Buffer *buffer)
{
    assert(buffer->lines_len > 0);

    Line line = buffer->lines[0];
    BufferTape tape = {.buffer = buffer, .line = line};
    In_CacheChar(&tape);

    return tape;
}

BufferTape BufferTape_InitAt(Buffer *buffer, Cursor cursor)
{
    assert(buffer->lines_len > cursor.line);

    Line line = buffer->lines[cursor.line];
    uint32_t dummy = 0; 
    // Locate line
    for (int column = 0; column < cursor.column && (line.text += Utf8_Fetch(&dummy, line.text)); column++)
        ;

    BufferTape tape = {.buffer = buffer, .cursor = cursor, .line = line};
    In_CacheChar(&tape);

    return tape;
}

int BufferTape_Next(BufferTape *tape)
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
    In_CacheChar(tape);
    return oldch;
}

int BufferTape_Get(BufferTape *tape) 
{
    return tape->ch;
}

