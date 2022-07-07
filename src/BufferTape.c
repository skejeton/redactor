#include <stdint.h>
#include <assert.h>
#include "BufferTape.h"

BufferTape BufferTape_Init(Buffer *buffer)
{
    assert(buffer->lines_len > 0);

    BufferTape tape = {.buffer = buffer, .line = buffer->lines[0].text};
    BufferTape_CacheChar(&tape);

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

    BufferTape tape = {.buffer = buffer, .cursor = cursor, .line = line.text};
    BufferTape_CacheChar(&tape);

    return tape;
}


