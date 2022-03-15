#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "buffer.h"
#include "utf8.h"

#define LINE_REALLOC_PERIOD 1024
#define LINES_REALLOC_PERIOD 32


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

static int utf8_strlen(const char *l) 
{
    int len = strlen(l);
    int ulen = 0;
    while (utf8_get(&l, &len))
        ulen++;
    return ulen;
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
    line->length = utf8_strlen(line->data);
}

static struct buffer_marker sanitize_marker(struct buffer *buffer, struct buffer_marker marker)
{
    if (marker.line < 0)
        marker.line = 0;
    else if (marker.line >= buffer->line_count)
        marker.line = buffer->line_count-1;

    int ulen = utf8_strlen(buffer->lines[marker.line].data);

    if (marker.column < 0)
        marker.column = 0;
    else if (marker.column > ulen)
        marker.column = ulen;
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

static struct buffer_marker utify_marker(struct buffer *buffer, struct buffer_marker marker)
{
    const char *line = buffer->lines[marker.line].data;
    const char *line_original = line;
    int max = buffer->lines[marker.line].size;

    while (marker.column-- && utf8_get(&line, &max)) 
        ;

    marker.column = line - line_original;
    return marker;
}

// Gives correct offsets from UTF-8 text
static struct buffer_range utify_range(struct buffer *buffer, struct buffer_range range)
{
    range.from = utify_marker(buffer, range.from);
    range.to   = utify_marker(buffer, range.to);
    return range;
}

struct buffer_range buffer_sanitize_range(struct buffer *buffer, struct buffer_range range)
{
    return sanitize_range(buffer, range);
}

struct buffer_marker buffer_sanitize_marker(struct buffer *buffer, struct buffer_marker marker)
{
    return sanitize_marker(buffer, marker);
}

struct buffer_range buffer_swap_ranges(struct buffer_range range) 
{
    return swap_ranges(range);
}

bool buffer_markers_equal(struct buffer_marker a, struct buffer_marker b)
{
    return a.line == b.line && a.column == b.column;
}

bool buffer_range_empty(struct buffer_range range)
{
    return buffer_markers_equal(range.from, range.to);
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
    struct buffer_range return_range = sanitize_range(buffer, range);
    range = utify_range(buffer, return_range);
    if (range.from.line == range.to.line) {
        erase_from_line(&buffer->lines[range.from.line], range.from.column, range.to.column);
    } else {
        buffer->lines[range.from.line].data[range.from.column] = 0;
        buffer->lines[range.from.line].size = range.from.column;
        line_write_at(&buffer->lines[range.from.line], range.from.column, buffer->lines[range.to.line].data+range.to.column);

        for (int i = range.from.line+1; i <= range.to.line; ++i) {
            remove_line(buffer, range.from.line+1);
        } 
    }
    return return_range.from;
}

// FIXME: SLOW!
struct buffer_marker buffer_insert(struct buffer *buffer, struct buffer_marker marker, const char *text)
{
    buffer->dirty = true;
    struct buffer_marker test_marker = sanitize_marker(buffer, marker);
    marker = utify_marker(buffer, test_marker);
    int sl = utf8_strlen(text);
    char c;
    while ((c = *text++)) {
        if (c == '\n') {
            int pl = marker.line;
            char *tail = buffer->lines[marker.line].data+marker.column;
            insert_line(buffer, ++marker.line);
            line_write_at(&buffer->lines[marker.line], 0, tail);
            buffer->lines[pl].data[marker.column] = 0;
            buffer->lines[pl].size = marker.column;
            marker.column = 0;
            sl = utf8_strlen(text);
        }
        else {
            char s[] = {c, 0};
            line_write_at(&buffer->lines[marker.line], marker.column++, s);
        }
    }
    
    if (test_marker.line == marker.line)
        marker.column = test_marker.column+sl;
    else 
        marker.column = sl;
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
    int overline = swap_ranges(range).to.line > buffer->line_count;
    range = utify_range(buffer, sanitize_range(buffer, range));

    int range_len = range_length(buffer, range);
    int buf_len = range_len+(range.to.line-range.from.line)+2;
    char *buf = malloc(buf_len);
    *buf = 0;
    char *cur = buf;
    
    if (range.from.line == range.to.line) {
        cur = strncat(buf, buffer->lines[range.from.line].data+range.from.column, range_len);
    } else {
        cur = strcat(cur, buffer->lines[range.from.line].data+range.from.column);
        cur = strcat(cur, "\n");

        for (int i = range.from.line+1; i < range.to.line; ++i) {
            cur = strcat(cur, buffer->lines[i].data);
            cur = strcat(cur, "\n");
        }
        cur = strncat(cur, buffer->lines[range.to.line].data,  range.to.column);
    }
    if (overline) 
        buf[buf_len-2] = '\n'; 
    buf[buf_len-1] = 0; 
    return buf;
}

int buffer_get_char(struct buffer *buffer, struct buffer_marker at)
{
    if (at.line < 0)
        return 0;
    if (at.line >= buffer->line_count)
        return 0;
    if (at.column < 0)
        return 0;
    if (at.column > buffer->lines[at.line].size)
        return 0;
    at = utify_marker(buffer, at);
    int max = buffer->lines[at.line].size-at.column;
    const char *l = &buffer->lines[at.line].data[at.column];
    return utf8_get(&l, &max);
}

struct buffer buffer_init() 
{
    struct buffer buffer = { 0 };
    insert_line(&buffer, 0);
    return buffer;
}

struct buffer_marker buffer_move_marker_(struct buffer *buffer, struct buffer_marker marker, int hor, int ver) 
{
    if (marker.column == 0 && hor < 0 && marker.line > 0) {
        marker = buffer_move_marker_(buffer, marker, 0, -1);
        marker.column = buffer->lines[marker.line].length;
        marker = buffer_move_marker_(buffer, marker, hor+1, 0);
        return marker;
    } else if (marker.line < buffer->line_count-1 && marker.column == buffer->lines[marker.line].length && hor > 0) {
        marker = buffer_move_marker_(buffer, marker, 0, 1);
        marker.column = 0;
        marker = buffer_move_marker_(buffer, marker, hor-1, 0);
        return marker;
    } else {
        marker.column += hor;
        marker.line += ver;
        return sanitize_marker(buffer, marker);
    }
}

struct buffer_marker buffer_move_marker(struct buffer *buffer, struct buffer_marker marker, int hor, int ver) 
{
    return buffer_move_marker_(buffer, marker, hor, ver);
}


void buffer_deinit(struct buffer *buffer)
{
    for (int i = 0; i < buffer->line_count; i += 1)
        free(buffer->lines[i].data);
    free(buffer->lines);
}


int buffer_line_length(struct buffer *buffer, int line)
{
    if (line >= 0 && line < buffer->line_count) {
        return utf8_strlen(buffer->lines[line].data);
    } else {
        return 0;
    }
}

struct buffer_range buffer_marker_pretext_range(struct buffer *buffer, struct buffer_marker marker)
{
    marker = sanitize_marker(buffer, marker);
    return (struct buffer_range){{marker.line, 0}, {marker.line, marker.column}};
}

struct buffer_range buffer_marker_posttext_range(struct buffer *buffer, struct buffer_marker marker)
{
    marker = sanitize_marker(buffer, marker);
    int line_posttext_column = buffer_line_length(buffer, marker.line);
    return (struct buffer_range){{marker.line, marker.column}, {marker.line, line_posttext_column}};
}

int buffer_marker_cmp(struct buffer_marker a, struct buffer_marker b)
{
    if (a.line == b.line && a.column == b.column)
        return 0;
    else if ((a.line == b.line && a.column < b.column) || (a.line < b.line))
        return -1;
    else
        return 1;
}
