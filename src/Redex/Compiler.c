#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "src/Utf8.h"
#include "Redex.h"

#define REALLOC_PERIOD 16

typedef struct {
    const char *source;
} In_Compiler;

static void In_AddSubgroup(Redex_Group *group, Redex_SubGroup subgroup)
{
    if (group->subgroups_len % REALLOC_PERIOD == 0) {
        group->subgroups = realloc(group->subgroups, sizeof(*group->subgroups) * (group->subgroups_len+512));
    }

    group->subgroups[group->subgroups_len++] = subgroup;
}

static bool In_IsCompiling(In_Compiler *co)
{
    return *co->source;
}

static Redex_SubGroup In_CompileBasic(In_Compiler *co);

// NOTE(skejeton): This uses `)` as a sentinel.
static Redex_SubGroup In_CompileGroup(In_Compiler *co)
{
    Redex_Group out = {0};
    while (In_IsCompiling(co) && *co->source != ')') {
        In_AddSubgroup(&out, In_CompileBasic(co));
    }
    
    // NOTE(skejeton): Skip `)` sentinel
    if (In_IsCompiling(co)) {
        co->source++;
    }

    return (Redex_SubGroup){0, Redex_SubGroup_Group, {.group = out}};
}

// NOTE(skejeton): This uses `]` as a sentinel.
static Redex_SubGroup In_CompileCharset(In_Compiler *co)
{
    Redex_Group out = {0};
    while (In_IsCompiling(co) && *co->source != ')') {
        In_AddSubgroup(&out, In_CompileBasic(co));
    }
    
    // NOTE(skejeton): Skip `)` sentinel
    if (In_IsCompiling(co)) {
        co->source++;
    }   

    return (Redex_SubGroup){0, Redex_SubGroup_Group, {.group = out}};
}

static Redex_SubGroup In_CompileBasic(In_Compiler *co)
{
    Redex_SubGroup out;

    switch (*co->source) {
    case '(':
        co->source++;
        out = In_CompileGroup(co);
        break;
    case '[':
        co->source++;
        out = In_CompileCharset(co);
        break;
    default:
        out = (Redex_SubGroup){0, Redex_SubGroup_Char, {.ch = Utf8_NextVeryBad(&co->source)}};
        break;
    }

    switch (*co->source) {
    case '*':
        out.quantifier = Redex_Quantifier_All;
        co->source += 1;
        break;
    case '+':
        out.quantifier = Redex_Quantifier_Greedy;
        co->source += 1;
        break;
    case '?':
        out.quantifier = Redex_Quantifier_Lazy;
        co->source += 1;
        break;
    default:
        out.quantifier = Redex_Quantifier_None;
        break;
    }

    return out;
}

Redex_CompiledExpression Redex_Compile(const char *redex)
{
    In_Compiler co = {redex};
    Redex_Group out = {0};
    
    while (In_IsCompiling(&co)) {
        In_AddSubgroup(&out, In_CompileBasic(&co));
    }

    return out;
}
