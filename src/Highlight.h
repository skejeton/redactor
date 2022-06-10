#ifndef R_HIGHLIGHT_H
#define R_HIGHLIGHT_H

#include <SDL2/SDL.h>
#include "BufferDraw.h"

enum {
    Highlight_Rule_AnyKw,
    Highlight_Rule_Wrapped,
    Highlight_Rule_Redex,
    Highlight_Rule_Lookahead,
};

struct {
    int rule_type;
    SDL_Color color;
    union {
        const char **rule_anykw;
        struct { 
            const char *begin, *end, *slash;
        } rule_wrapped;
        const char *rule_redex;
        struct { 
            const char *data, *tail;
        } rule_lookahead;
    };
}
typedef Highlight_Rule;

struct {
    Highlight_Rule *rules;
    int rules_count;
}
typedef Highlight_Set;

void Highlight_HighlightBuffer(Buffer *buf, const Highlight_Set *set, BufferDrawSegments *out_segments);

#endif
