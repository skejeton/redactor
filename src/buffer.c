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
    free(buffer->lines[at].data);
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


static struct buffer_marker sanitize_marker(struct buffer *buffer, struct buffer_marker marker)
{
    if (marker.line < 0)
        marker.line = 0;
    else if (marker.line >= buffer->line_count)
        marker.line = buffer->line_count-1;
    if (marker.column < 0)
        marker.column = 0;
    else if (marker.column > buffer->lines[marker.line].size)
        marker.column = buffer->lines[marker.line].size;
    return marker;
}

static struct buffer_range swap_ranges(struct buffer_range range)
{
    struct buffer_marker tmp;

    if ((range.from.line > range.to.line) || 
        ((range.from.line == range.to.line) && (range.from.column > range.to.column))) {
        tmp = range.from;
        range.from = range.to;
        range.to = tmp;
    }
    return range;
}

static struct buffer_range sanitize_range(struct buffer *buffer, struct buffer_range range)
{
    range.from = sanitize_marker(buffer, range.from);
    range.to = sanitize_marker(buffer, range.to);
    
    range = swap_ranges(range);
    return range;
}

static void erase_from_line(struct buffer_line *line, int from, int to)
{
    memmove(line->data+from, line->data+to, line->size-to);
    line->size -= to-from;
    line->data[line->size] = 0;
}

struct buffer_marker buffer_remove(struct buffer *buffer, struct buffer_range range)
{
    buffer->dirty = true;
    range = sanitize_range(buffer, range);
    if (range.from.line == range.to.line) {
        erase_from_line(&buffer->lines[range.from.line], range.from.column, range.to.column);
    } else {
        buffer->lines[range.from.line].data[range.from.column] = 0;
        line_write_at(&buffer->lines[range.from.line], range.from.column, buffer->lines[range.to.line].data+range.to.column);

        for (int i = range.from.line+1; i <= range.to.line; ++i) {
            remove_line(buffer, range.from.line+1);
        } 
    }
    return range.from;
}

// FIXME: SLOW!
struct buffer_marker buffer_insert(struct buffer *buffer, struct buffer_marker marker, const char *text)
{
    buffer->dirty = true;
    marker = sanitize_marker(buffer, marker);
    
    while (*text) {
        char c = *text++;
        if (c == '\n') {
            int pl = marker.line;
            char *tail = buffer->lines[marker.line].data+marker.column;
            insert_line(buffer, ++marker.line);
            line_write_at(&buffer->lines[marker.line], 0, tail);
            buffer->lines[pl].data[marker.column] = 0;
            buffer->lines[pl].size = marker.column;
            marker.column = 0;
        }
        else {
            line_put_at(&buffer->lines[marker.line], marker.column++, c);
        }
    }
    
    return marker;
}

static int range_length(struct buffer *buffer, struct buffer_range range)
{
    if (range.from.line == range.to.line) {
        return range.to.column - range.from.column;
    }
    else {
        int length = buffer->lines[range.from.line].size-range.from.column+range.to.column;
        for (int i = range.from.line+1; i < range.to.line; i++) {
            length += buffer->lines[i].size;
        }
        return length;
    }
}

// FIXME: This function is atrocious
char *buffer_get_range(struct buffer *buffer, struct buffer_range range)
{
    range = swap_ranges(range);
    int origcol = range.to.column;
    range = sanitize_range(buffer, range);
    int nl = origcol > buffer->lines[range.from.line].size;
    int range_len = range_length(buffer, range);
    int buf_len = range_len+(range.to.line-range.from.line)+2;
    char *buf = malloc(buf_len);
    *buf = 0;
    char *cur = buf;
    
    if (range.from.line == range.to.line) {
        cur = strncat(buf, buffer->lines[range.from.line].data+range.from.column, range_len);
        if (nl)
            cur = strcat(cur, "\n");
    } else {
        cur = strcat(cur, buffer->lines[range.from.line].data+range.from.column);
        cur = strcat(cur, "\n");

        for (int i = range.from.line+1; i < range.to.line; ++i) {
            cur = strcat(cur, buffer->lines[i].data);
            cur = strcat(cur, "\n");
        }
        cur = strncat(cur, buffer->lines[range.to.line].data,  buffer->lines[range.to.line].size);
        if (nl)
            cur = strcat(cur, "\n");
    }
    buf[buf_len-1] = 0; 
    return buf;
}

// buffer_replace (accepts a range)
// buffer_get (accepts a range, returns the string)

struct buffer buffer_init() 
{
    struct buffer buffer = { 0 };
    insert_line(&buffer, 0);
    return buffer;
}


struct buffer_marker buffer_move_marker(struct buffer *buffer, struct buffer_marker marker, int hor, int ver) 
{
    if (marker.column == 0 && hor < 0 && marker.line > 0) {
        marker = buffer_move_marker(buffer, marker, 0, -1);
        marker.column = buffer->lines[marker.line].size;
        marker = buffer_move_marker(buffer, marker, hor+1, 0);
        return marker;
    }
    else if (marker.column == buffer->lines[marker.line].size && hor > 0 && marker.line < buffer->line_count-1) {
        marker = buffer_move_marker(buffer, marker, 0, 1);
        marker.column = 0;
        marker = buffer_move_marker(buffer, marker, hor-1, 0);
        return marker;
    }
    else {
        marker.column += hor;
        marker.line += ver;
        marker = sanitize_marker(buffer, marker);
        return marker;
    }
}


void buffer_deinit(struct buffer *buffer)
{
    for (int i = 0; i < buffer->line_count; i += 1)
        free(buffer->lines[i].data);
}
