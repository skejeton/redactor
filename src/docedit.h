#ifndef REDACTOR_DOCEDIT_H
#define REDACTOR_DOCEDIT_H 
#include "buffer.h"

struct docedit {
    struct buffer buffer;
    struct docedit_cursor {
        struct buffer_range selection;
    } cursor;
};

void docedit_insert(struct docedit *docedit, const char *text);
void docedit_set_cursor(struct docedit *docedit, bool select, struct buffer_marker marker);
void docedit_move_cursor(struct docedit *docedit, bool select, int hor, int ver);
void docedit_erase(struct docedit *docedit); 
char *docedit_get_selection(struct docedit *docedit);

#endif
