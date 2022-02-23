#ifndef REDACTOR_BUFFER_H
#define REDACTOR_BUFFER_H
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct buffer_cursor {
    int remcol; 
    int column;
    int line;
};

struct buffer_line {
    char *data;
    int size;
};

struct buffer {
    bool dirty;
    struct buffer_cursor cursor;
    struct buffer_line *lines;
    size_t line_count;
};

struct buffer buffer_init();
char *buffer_get_trimmed_line_at(struct buffer *buffer, int index);
void buffer_insert_character(struct buffer *buffer, int chara);
void buffer_write(struct buffer *buffer, char *s);
void buffer_erase_character(struct buffer *buffer);
void buffer_move(struct buffer *buffer, int hor, int ver);
char *buffer_get_whole_file(struct buffer *buffer);
void buffer_deinit(struct buffer *buffer);
#endif
