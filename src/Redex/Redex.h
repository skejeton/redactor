#ifndef R_REDEX_H
#define R_REDEX_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "src/Utf8.h"

enum {
    Redex_SubGroup_Char,
    Redex_SubGroup_Group,
    Redex_SubGroup_Charset,
    Redex_SubGroup_CharacterClass,
}
typedef Redex_SubGroup_Type;

enum {
    Redex_Quantifier_None,     //
    Redex_Quantifier_All,      // *
    Redex_Quantifier_Greedy,   // +
    Redex_Quantifier_Lazy      // ?
}
typedef Redex_Quantifier;

enum {
    Redex_CharacterClass_Any   // .
}
typedef Redex_CharacterClass;

// Character range (Inclusive)
struct {
    uint32_t from;
    uint32_t to;
}
typedef Redex_CharacterRange;

struct {
    Redex_CharacterRange *ranges;
    size_t ranges_len;
    bool inverted;  // NOTE(skejeton): for checks if characters do not match
}
typedef Redex_Charset;

struct Redex_Group {
    struct Redex_SubGroup *subgroups;
    size_t subgroups_len;
}
typedef Redex_Group;

struct Redex_SubGroup {
    Redex_Quantifier quantifier;
    Redex_SubGroup_Type type;
    union {
        uint32_t ch;
        Redex_Group group;
        Redex_Charset charset;
        Redex_CharacterClass character_class;
    };
}
typedef Redex_SubGroup;

typedef Redex_Group Redex_CompiledExpression;

Redex_CompiledExpression Redex_Compile(const char *redex);
void Redex_CompiledExpressionDeinit(Redex_CompiledExpression *expr);

#endif
