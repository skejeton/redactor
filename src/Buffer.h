#ifndef R_BUFFER_H
#define R_BUFFER_H

#include <stdlib.h>
#include <stdbool.h>

struct {
    // NOTE: text is a utf8 string
    char *text;
    // NOTE: text_size is the size in bytes
    size_t text_size;
    // NOTE: text_len is the size in runes
    size_t text_len;
}
typedef Line;

struct {
    int32_t line;
    int32_t column;
} 
typedef Cursor;

struct {
    Line *lines;
    size_t lines_len;
}
typedef Buffer;

struct {
    Cursor from, to;
}
typedef Range;

Cursor Buffer_InsertUTF8(Buffer *buf, Cursor cursor, const char *text);
Cursor Buffer_RemoveCharacterUnder(Buffer *buf, Cursor under);
Cursor Buffer_MoveCursor(Buffer *buf, Cursor cursor, int lines, int cols);
Cursor Buffer_EndCursor(Buffer *buf);
// a < b  => -1
// a == b =>  0
// a > b  =>  1
static inline int Buffer_CompareCursor(Cursor a, Cursor b) {
    if (a.line == b.line) {
        return (a.column < b.column) * -1 + (a.column > b.column) * 1;
    } else {
        return (a.line < b.line) * -1 + (a.line > b.line) * 1;
    }
}

static inline bool Buffer_CursorInRange(Cursor cur, Range range) {
    if (cur.line == range.from.line && range.from.line == range.to.line) {
        return cur.column >= range.from.column && cur.column < range.to.column;
    } else if (cur.line == range.from.line) {
        return cur.column >= range.from.column;
    } else if (cur.line == range.to.line) {
        return cur.column < range.to.column;
    } else if (cur.line > range.from.line && cur.line < range.to.line) {
        return true;
    } 

    return false;
}

char *Buffer_GetStringRange(Buffer *buf, Range range);
Buffer Buffer_Init();
Buffer Buffer_InitFromString(const char *s);
void Buffer_Deinit(Buffer *buf);

#endif
