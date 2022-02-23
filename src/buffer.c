#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "buffer.h"

#define LINE_REALLOC_PERIOD 1024
#define LINES_REALLOC_PERIOD 32

void buffer_move(struct buffer *buffer, int hor, int ver);

static void add_line(struct buffer *buffer)
{
    if (buffer->line_count % LINES_REALLOC_PERIOD == 0) {
        buffer->lines = realloc(buffer->lines, sizeof(struct buffer_line)*(buffer->line_count+LINES_REALLOC_PERIOD));
    }
    buffer->line_count += 1;
}

static void insert_line(struct buffer *buffer, int at)
{
    add_line(buffer);
    memmove(buffer->lines+at+1, buffer->lines+at, sizeof(struct buffer_line)*(buffer->line_count-at-1));
    buffer->lines[at].data = calloc(1, LINE_REALLOC_PERIOD+1 /* plus sentinel */);
    buffer->lines[at].size = 0;
}

static void remove_line(struct buffer *buffer, int at)
{
    memmove(buffer->lines+at, buffer->lines+at+1, sizeof(struct buffer_line)*(buffer->line_count-at-1));
    buffer->line_count -= 1;
}

static void line_put_at(struct buffer_line *line, int at, int c)
{
    if (line->size % LINE_REALLOC_PERIOD == 0)
        line->data = realloc(line->data, line->size+LINE_REALLOC_PERIOD+1);
    memmove(line->data+at+1, line->data+at, line->size-at);
    line->data[at] = c;
    line->data[++line->size] = 0;
}

static void line_write_at(struct buffer_line *line, int at, const char *s) 
{
    while (*s)
        line_put_at(line, at++, *s++);
}

struct buffer buffer_init() 
{
    struct buffer buffer = { 0 };
    insert_line(&buffer, 0);
    return buffer;
}

char *buffer_get_whole_file(struct buffer *buffer)
{
    int sz = 1;
    for (int i = 0; i < buffer->line_count; i += 1) {
        sz += buffer->lines[i].size+1; // account for newline 
    }

    char *f = malloc(sz);
    *f = 0;
    for (int i = 0; i < buffer->line_count; i += 1) {
        strcat(f, buffer->lines[i].data);
        strcat(f, "\n");
    }
    return f;
}

char *buffer_get_trimmed_line_at(struct buffer *buffer, int index) 
{
    if (index < 0) index = 0;
    struct buffer_line current_line = buffer->lines[buffer->cursor.line];

    if (index > current_line.size) 
        index = current_line.size;

    char *s = malloc(index+1);
    memcpy(s, current_line.data, index);
    s[index] = 0;

    return s;
}

void buffer_move(struct buffer *buffer, int hor, int ver) 
{
    struct buffer_line current_line = buffer->lines[buffer->cursor.line];
    if (buffer->cursor.column == 0 && hor < 0 && buffer->cursor.line > 0) {
        buffer_move(buffer, 0, -1);
        struct buffer_line current_line = buffer->lines[buffer->cursor.line];
        buffer->cursor.column = buffer->cursor.remcol = current_line.size;
        buffer_move(buffer, hor+1, 0);
        return;
    }
    if (buffer->cursor.column == current_line.size && hor > 0 && buffer->cursor.line < buffer->line_count-1) {
        buffer_move(buffer, 0, 1);
        buffer->cursor.column = buffer->cursor.remcol = 0;
        buffer_move(buffer, hor-1, 0);
        return;
    }

    if (hor == 0)
        buffer->cursor.column = buffer->cursor.remcol;
    else
        buffer->cursor.remcol = buffer->cursor.column += hor;

    buffer->cursor.line += ver;
    if (buffer->cursor.line < 0) buffer->cursor.line = 0;
    if (buffer->cursor.line >= buffer->line_count) buffer->cursor.line = buffer->line_count-1;
    current_line = buffer->lines[buffer->cursor.line];
    if (buffer->cursor.column < 0) buffer->cursor.column = 0;
    if (buffer->cursor.column > current_line.size) buffer->cursor.column = current_line.size;
}

void buffer_insert_character(struct buffer *buffer, int chara)
{   
    struct buffer_line *current_line = &buffer->lines[buffer->cursor.line];
    buffer->dirty = 1;
    if (chara == '\n') {
        insert_line(buffer, buffer->cursor.line+1);
        // reset due to realloc
        current_line = &buffer->lines[buffer->cursor.line];
        line_write_at(&buffer->lines[buffer->cursor.line+1], 0, current_line->data+buffer->cursor.column);
        current_line->data[buffer->cursor.column] = 0;
        current_line->size = buffer->cursor.column;
        buffer_move(buffer, 0, 1);
        buffer->cursor.column = 0;
    } else {
        line_put_at(current_line, buffer->cursor.column++, chara);
    }
}

void buffer_write(struct buffer *buffer, char *s)
{
    while (*s)
        buffer_insert_character(buffer, *s++);
}

void buffer_erase_character(struct buffer *buffer)
{
    struct buffer_line *current_line = &buffer->lines[buffer->cursor.line];
    buffer->dirty = 1;
    if (buffer->cursor.column == 0) {
        if (buffer->cursor.line > 0) {
            size_t at = buffer->lines[buffer->cursor.line-1].size;
            line_write_at(&buffer->lines[buffer->cursor.line-1], at, current_line->data);
            remove_line(buffer, buffer->cursor.line--);
            buffer->cursor.column = at;
        }
        return;
    }
    char *at = current_line->data+buffer->cursor.column;
    memmove(at-1, at, current_line->size---buffer->cursor.column--);
    current_line->data[current_line->size] = 0;
}

void buffer_deinit(struct buffer *buffer)
{
    for (int i = 0; i < buffer->line_count; i += 1)
        free(buffer->lines[i].data);
}
