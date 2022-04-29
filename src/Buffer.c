#include "Redactor.h"
#include "Unicode.h"
#include <stdlib.h>
#include <string.h>

Cursor InsertUTF8Solo(Buffer *buf, Cursor cursor, const char *text)
{
    Line *line = &buf->lines[cursor.line];
    size_t new_size = strlen(text) + line->text_size;
    char *new_line_start = malloc(sizeof(char) * (new_size + 1));
    char *new_line = new_line_start;
    int cursor_pos_byte = 0;
    int unilen = Uni_Utf8_Strlen(text);
    const char *line_iter = line->text;

        // --find cursor
    for (int i = 0; i != cursor.column && Uni_Utf8_NextVeryBad(&line_iter); ++i)
        ;
    cursor_pos_byte = line_iter - line->text;

        // --before
    for (int i = 0; i < cursor_pos_byte; ++i)
        *new_line++ = line->text[i];
        // --mid
    for (int i = 0; text[i]; ++i)
        *new_line++ = text[i];
        // --after
    for (int i = cursor_pos_byte; line->text[i]; ++i)
        *new_line++ = line->text[i];

    *new_line = 0;

    free(line->text);
    line->text = new_line_start;
    line->text_size = new_size;
    line->text_len += unilen;

    cursor.column += unilen;
    return cursor;
}

Cursor MoveCursorColumns(Buffer *buf, Cursor cursor, int col)
{
    int column = (int)(cursor.column) + col;

    if (column < 0) {
        while (column < 0 && cursor.line > 0) {
            cursor.line--;
            column += buf->lines[cursor.line].text_len+1;
        }
    } else {
        while (column > buf->lines[cursor.line].text_len && cursor.line < buf->lines_len-1) {
            column -= buf->lines[cursor.line].text_len+1;
            cursor.line++;
        }
    }
    if (column < 0) {
        column = 0; 
    } else if (column > buf->lines[cursor.line].text_len) {
        column = buf->lines[cursor.line].text_len;
    }

    cursor.column = column;


    return cursor;
}

void AddEmptyLineAt(Buffer *buf, Cursor under)
{
    if (buf->lines_len % 1024 == 0) {
        buf->lines = realloc(buf->lines, sizeof(*buf->lines) * (buf->lines_len + 1024));
    }

    memmove(buf->lines+under.line+1, buf->lines+under.line, sizeof(*buf->lines) * (buf->lines_len-under.line));
    char *empty = malloc(1);
    empty[0] = 0;

    buf->lines[under.line].text_len = 0;
    buf->lines[under.line].text_size = 0;
    buf->lines[under.line].text = empty;
    buf->lines_len++;
}

void AddLine(Buffer *buf, const char *line)
{
    if (buf->lines_len % 1024 == 0) {
        buf->lines = realloc(buf->lines, sizeof(*buf->lines) * (buf->lines_len + 1024));
    }

    size_t text_size = strlen(line);
    char *text = strcpy(malloc(text_size+1), line);

    buf->lines[buf->lines_len  ].text_len  = Uni_Utf8_Strlen(text);
    buf->lines[buf->lines_len  ].text_size = text_size;
    buf->lines[buf->lines_len++].text      = text;
}

Cursor SplitLineAt(Buffer *buf, Cursor under)
{
    Line *line = &buf->lines[under.line];
    line->text_len = under.column;

    char *line_text = line->text;
    const char *line_iter = line->text;
        // NOTE: character before cursor

        // --find cursor
    for (int i = 0; i != under.column && Uni_Utf8_NextVeryBad(&line_iter); ++i)
        ;
    line->text_size = line_iter-line->text;

    under.line += 1;
    under.column = 0;
    AddEmptyLineAt(buf, under);
    Buffer_InsertUTF8(buf, under, line_iter);
    line_text[line_iter-line_text] = 0;
    return under;
}

Cursor Buffer_RemoveCharacterUnder(Buffer *buf, Cursor under)
{
    if (under.column == 0) {
        if (under.line > 0) {
            under.line--;
                        // TODO: Inline this with a specialized version
            Line *line = &buf->lines[under.line];
            under.column = line->text_len;
            Buffer_InsertUTF8(buf, under, buf->lines[under.line+1].text);

            free(buf->lines[under.line+1].text);
            memmove(buf->lines+under.line+1, buf->lines+under.line+2, sizeof(*buf->lines) * (buf->lines_len - (under.line+2)));
            buf->lines_len -= 1;
        }
    } else {
        Line *line = &buf->lines[under.line];

        const char *line_iter = line->text;
                // NOTE: character before cursor
        const char *p_line_iter = line->text;

                // --find cursor
        for (int i = 0; i != under.column && Uni_Utf8_NextVeryBad((p_line_iter = line_iter, &line_iter)); ++i)
            ;

        memmove(line->text + (p_line_iter - line->text), line->text + (line_iter - line->text), sizeof(char) * (line->text_size - (line_iter - line->text) + 1));

        line->text_len -= 1;

        under.column -= 1;
    }
    return under;
}

Cursor Buffer_MoveCursor(Buffer *buf, Cursor cursor, int lines, int cols)
{
    cursor.line += lines;

    if (cursor.line < 0) {
        cursor.line = 0;
    } else if (cursor.line >= buf->lines_len) {
        cursor.line = buf->lines_len-1;
    }

    if (cursor.column < 0) {
        cursor.column = 0;
    } else if (cursor.column > buf->lines[cursor.line].text_len) {
        cursor.column = buf->lines[cursor.line].text_len;
    }

    return MoveCursorColumns(buf, cursor, cols);
}

Cursor Buffer_InsertUTF8(Buffer *buf, Cursor cursor, const char *text)
{
    char *txt = Util_Strdup(text);
    char *txtStart = txt;
    while (*txt) {
        char *start = txt;
        while (*txt && *txt != '\n') {
            txt++;
        }
        int c = *txt;
        *txt = 0;
        
        cursor = InsertUTF8Solo(buf, cursor, start);
        *txt = c;

        if (*txt) {
            cursor = SplitLineAt(buf, cursor);
            txt++;
        }
    }
    free(txtStart);
    return cursor;
}

Buffer Buffer_Init()
{
    Buffer buf = {0};
    AddLine(&buf, "");
    return buf;
}

void Buffer_Deinit(Buffer *buf)
{
    for (int i = 0; i < buf->lines_len; ++i) {
        free(buf->lines[i].text);
    }
    free(buf->lines);
}

