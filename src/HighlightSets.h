#ifndef R_HIGHLIGHTSETS_H
#define R_HIGHLIGHTSETS_H

#include "Highlight.h"

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
typedef HighlightSets_Rule;

struct {
    HighlightSets_Rule *rules;
    int rules_len;
}
typedef HighlightSets_Set;

extern const HighlightSets_Set HighlightSets_C;

Highlight_Set HighlightSets_Compile(const HighlightSets_Set *set);

#endif
