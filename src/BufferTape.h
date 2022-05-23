#ifndef BUFFERTAPE_H
#define BUFFERTAPE_H

#include <stdint.h>
#include "Buffer.h"

struct {
    Buffer *buffer;
    Cursor cursor;
    uint32_t ch;
    Line line;
}
typedef BufferTape;

BufferTape BufferTape_Init(Buffer *buffer);
BufferTape BufferTape_InitAt(Buffer *buffer, Cursor cursor);
int BufferTape_Next(BufferTape *tape); // Returns current character, advances the tape
int BufferTape_Get(BufferTape *tape); // Returns current character at the tape position

#endif
