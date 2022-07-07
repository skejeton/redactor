#ifndef R_HIGHLIGHT_H
#define R_HIGHLIGHT_H

#include <SDL2/SDL.h>
#include "BufferDraw.h"
#include "Redex/Redex.h"

enum {
    Highlight_Rule_AnyKw,
    Highlight_Rule_Wrapped,
    Highlight_Rule_Redex,
    Highlight_Rule_Lookahead,
}
typedef Highlight_RuleType;

struct {
    Highlight_RuleType rule_type;
    SDL_Color color;
    union {
        struct {
            char **keywords;
        } rule_anykw;
        struct { 
            Redex_CompiledExpression begin, end, slash;
        } rule_wrapped;
        Redex_CompiledExpression rule_redex;
        struct { 
            Redex_CompiledExpression data, tail;
        } rule_lookahead;
    };
}
typedef Highlight_Rule;

struct {
    Highlight_Rule *rules;
    int rules_len;
}
typedef Highlight_Set;

void Highlight_HighlightBuffer(Buffer *buf, const Highlight_Set *set, BufferDrawSegments *out_segments);
void Highlight_HighlightSetDeinit(Highlight_Set *set);

#endif
