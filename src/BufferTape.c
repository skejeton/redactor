#include <stdint.h>
#include <assert.h>
#include "BufferTape.h"
#include "Utf8.h"

BufferTape BufferTape_Init(Buffer *buffer)
{
    assert(buffer->lines_len > 0);

    Line line = buffer->lines[0];
    uint32_t ch;
    line.text += Utf8_Fetch(&ch, buffer->lines[0].text);

    return (BufferTape){.buffer = buffer, .ch = ch, .line = line};
}

BufferTape BufferTape_InitAt(Buffer *buffer, Cursor cursor)
{
    assert(buffer->lines_len > cursor.line);
    
    Line line = buffer->lines[cursor.line];
    uint32_t ch = 0; 
    // Locate line
    for (int column = 0; column <= cursor.column && (line.text += Utf8_Fetch(&ch, line.text)); column++)
        ;

    return (BufferTape){.buffer = buffer, .cursor = cursor, .ch = ch, .line = line};
}

int BufferTape_Next(BufferTape *tape)
{
    if (*tape->line.text == 0 && tape->cursor.line < tape->buffer->lines_len-1) {
        uint32_t oldch = tape->ch;
        tape->line = tape->buffer->lines[tape->cursor.line+1];
        tape->cursor.column += 1;
        tape->ch = '\n';
        return oldch;
    } else {
        uint32_t oldch = tape->ch;
        uint32_t newch;
        tape->line.text += Utf8_Fetch(&newch, tape->line.text);
        switch (oldch) {
        case '\0': break;
        case '\n': tape->cursor.line += 1; tape->cursor.column = 0; break;
        default: tape->cursor.column += 1; break;
        }
        tape->ch = newch;
        return oldch;
    }
}

int BufferTape_Get(BufferTape *tape) 
{
    return tape->ch;
}

