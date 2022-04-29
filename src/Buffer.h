#include <stdlib.h>

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


Cursor Buffer_InsertUTF8(Buffer *buf, Cursor cursor, const char *text);
Cursor Buffer_RemoveCharacterUnder(Buffer *buf, Cursor under);
Cursor Buffer_MoveCursor(Buffer *buf, Cursor cursor, int lines, int cols);
Buffer Buffer_Init();
void Buffer_Deinit(Buffer *buf);
