#include <stdint.h>

enum syn_token_style {
    SYN_TS_NORMAL,  
};

struct syn_marker {
    size_t line, column;
};

struct syn_token {
    enum syn_token_style style;
    struct syn_marker start;
    struct syn_marker end;
};

struct syn_record {
    struct syn_token *tokens;
    size_t token_count;
};

struct syn_stream {
    struct syn_record record;
    struct syn_marker cursor;
};

void syn_record_insert(struct syn_record *record, struct syn_token token);

// The difference from syn_record_insert is that this inserts a 
// token and accordingly puts the ending line/column pairs
void syn_stream_insert(struct syn_stream *stream, struct syn_token token);

// IMPROVEMENT: Use different type for character?
int syn_stream_peek(struct syn_stream *stream);

// Returns the current character, and moves on to the next one, returns EOF on end of file
int syn_stream_next(struct syn_stream *stream);


