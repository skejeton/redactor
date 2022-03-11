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
    // UTF8 Length
    int length;
};

struct buffer {
    bool dirty;
    struct buffer_line *lines;
    size_t line_count;
};

struct buffer buffer_init();
void buffer_deinit(struct buffer *buffer);

struct buffer_range buffer_sanitize_range(struct buffer *buffer, struct buffer_range range);
struct buffer_marker buffer_sanitize_marker(struct buffer *buffer, struct buffer_marker marker);
struct buffer_range buffer_swap_ranges(struct buffer_range range);
bool buffer_markers_equal(struct buffer_marker a, struct buffer_marker b);
bool buffer_range_empty(struct buffer_range range);

struct buffer_marker buffer_remove(struct buffer *buffer, struct buffer_range range);
struct buffer_marker buffer_insert(struct buffer *buffer, struct buffer_marker marker, const char *text);
struct buffer_marker buffer_move_marker(struct buffer *buffer, struct buffer_marker marker, int hor, int ver);
char *buffer_get_range(struct buffer *buffer, struct buffer_range range);
int buffer_get_char(struct buffer *buffer, struct buffer_marker at);
int buffer_line_length(struct buffer *buffer, int line);
struct buffer_range buffer_marker_pretext_range(struct buffer *buffer, struct buffer_marker marker);
struct buffer_range buffer_marker_posttext_range(struct buffer *buffer, struct buffer_marker marker);
#endif

