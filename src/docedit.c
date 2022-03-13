#include "docedit.h"

void de_insert(struct docedit *docedit, const char *text)
{
    docedit->cursor.selection.to = docedit->cursor.selection.from = buffer_insert(&docedit->buffer, buffer_remove(&docedit->buffer, docedit->cursor.selection), text);
}

void de_set_cursor(struct docedit *docedit, bool select, struct buffer_marker marker)
{
    // Sanitize marker
    marker = buffer_move_marker(&docedit->buffer, marker, 0, 0);
    
    docedit->cursor.selection.to = marker;
    if (!select) {
        docedit->cursor.selection.from = marker;
    }
}

char *de_get_selection(struct docedit *docedit) 
{
    return buffer_get_range(&docedit->buffer, docedit->cursor.selection);
}

void de_move_cursor(struct docedit *docedit, bool select, int hor, int ver)
{
    de_set_cursor(docedit, select, buffer_move_marker(&docedit->buffer, docedit->cursor.selection.to, hor, ver));
}

void de_erase(struct docedit *docedit)
{
    if (buffer_range_empty(docedit->cursor.selection)) {
        docedit->cursor.selection.to = buffer_move_marker(&docedit->buffer, docedit->cursor.selection.to, -1, 0);
    }
    
    docedit->cursor.selection.from = docedit->cursor.selection.to = buffer_remove(&docedit->buffer, docedit->cursor.selection);
}

struct buffer_marker de_get_cursor(struct docedit *docedit)
{
    return docedit->cursor.selection.to;
}
