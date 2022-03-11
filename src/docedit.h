#ifndef REDACTOR_DOCEDIT_H
#define REDACTOR_DOCEDIT_H 
#include "buffer.h"

struct docedit {
    struct buffer buffer;
    struct docedit_cursor {
        struct buffer_range selection;
    } cursor;
};

void de_insert(struct docedit *docedit, const char *text);
void de_set_cursor(struct docedit *docedit, bool select, struct buffer_marker marker);
void de_move_cursor(struct docedit *docedit, bool select, int hor, int ver);
void de_erase(struct docedit *docedit); 
char *de_get_selection(struct docedit *docedit);
struct buffer_marker de_get_cursor(struct docedit *docedit);

#endif
