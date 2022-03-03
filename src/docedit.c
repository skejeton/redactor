#include "docedit.h"


void docedit_insert(struct docedit *docedit, const char *text)
{
    docedit->cursor.selection.to = docedit->cursor.selection.from = buffer_insert(&docedit->buffer, buffer_remove(&docedit->buffer, docedit->cursor.selection), text);
}


void docedit_set_cursor(struct docedit *docedit, bool select, struct buffer_marker marker)
{
    // Sanitize marker
    marker = buffer_move_marker(&docedit->buffer, marker, 0, 0);
    
    docedit->cursor.selection.from = marker;
    if (!select) {
        docedit->cursor.selection.to = marker;
    }
}


char *docedit_get_selection(struct docedit *docedit) 
{
    return buffer_get_range(&docedit->buffer, docedit->cursor.selection);
}


void docedit_move_cursor(struct docedit *docedit, bool select, int hor, int ver)
{
    docedit_set_cursor(docedit, select, buffer_move_marker(&docedit->buffer, docedit->cursor.selection.from, hor, ver));
}


void docedit_erase(struct docedit *docedit)
{
    if (buffer_range_empty(docedit->cursor.selection)) {
        docedit->cursor.selection.from = buffer_move_marker(&docedit->buffer, docedit->cursor.selection.from, -1, 0);
    }
    
    docedit->cursor.selection.from = docedit->cursor.selection.to = buffer_remove(&docedit->buffer, docedit->cursor.selection);
}
