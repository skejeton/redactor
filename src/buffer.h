#ifndef REDACTOR_BUFFER_H
#define REDACTOR_BUFFER_H
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct buffer_marker {
    int line;
    int column;
};

struct buffer_range {
    struct buffer_marker from;
    struct buffer_marker to;
};

struct buffer_line {
    char *data;
    int size;
};

struct buffer {
    bool dirty;
    struct buffer_line *lines;
    size_t line_count;
};

struct buffer buffer_init();
void buffer_deinit(struct buffer *buffer);

struct buffer_marker buffer_remove(struct buffer *buffer, struct buffer_range range);
struct buffer_marker buffer_insert(struct buffer *buffer, struct buffer_marker marker, const char *text);
struct buffer_marker buffer_move_marker(struct buffer *buffer, struct buffer_marker marker, int hor, int ver);
char *buffer_get_range(struct buffer *buffer, struct buffer_range range);
struct buffer_range buffer_swap_ranges(struct buffer_range range);
#endif

