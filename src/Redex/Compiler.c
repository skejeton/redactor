#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "src/Utf8.h"
#include "Redex.h"

#define REALLOC_PERIOD 16

typedef struct {
    const char *source;
    size_t ranges_len;
    size_t subgroups_len;
} In_Compiler;

static In_Compiler In_InitCompiler(const char *redex)
{
    In_Compiler co = {redex};
    return co;
}

static int32_t In_Fetch(In_Compiler *co)
{
    uint32_t new_char = 0;
    co->source += Utf8_Fetch(&new_char, co->source);

    if (new_char == '\\') {
        co->source += Utf8_Fetch(&new_char, co->source);

        switch (new_char) {
            case 't':
                return '\t';
            case 's':
                return ' ';
            case 'n':
                return '\n';
        }
    }

    return new_char;
}

static void In_AddSubgroup(In_Compiler *co, Redex_Group *group, Redex_SubGroup subgroup)
{
    if (group->subgroups_len % REALLOC_PERIOD == 0) {
        group->subgroups = realloc(group->subgroups, sizeof(*group->subgroups) * (group->subgroups_len+512));
    }

    co->subgroups_len += 1;
    group->subgroups[group->subgroups_len++] = subgroup;
}

static void In_AddRange(In_Compiler *co, Redex_Charset *set, Redex_CharacterRange range)
{
    if (set->ranges_len % REALLOC_PERIOD == 0) {
        set->ranges = realloc(set->ranges, sizeof(*set->ranges) * (set->ranges_len+512));
    }

    co->ranges_len += 1;
    set->ranges[set->ranges_len++] = range;
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
        In_AddSubgroup(co, &out, In_CompileBasic(co));
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
    Redex_Charset out = {0};

    if (*co->source == '^') {
        out.inverted = true;
        co->source++;
    }

    while (In_IsCompiling(co) && *co->source != ']') {
        Redex_CharacterRange range;
        range.from = In_Fetch(co);
        range.to = range.from;
        if (*co->source == '-') {
            co->source += 1;
            range.to = In_Fetch(co);
        }

        if (range.to < range.from) {
            continue;
        }

        In_AddRange(co, &out, range);
    }
    
    // NOTE(skejeton): Skip `]` sentinel
    if (In_IsCompiling(co)) {
        co->source++;
    }   

    return (Redex_SubGroup){0, Redex_SubGroup_Charset, {.charset = out}};
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
    case '.':
        out = (Redex_SubGroup){0, Redex_SubGroup_CharacterClass, {.character_class = Redex_CharacterClass_Any}};
        co->source++;
        break;
    default:
        out = (Redex_SubGroup){0, Redex_SubGroup_Char, {.ch = In_Fetch(co)}};
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

struct {
    size_t offset;
}
typedef In_MemoryMeta;

void* In_AllocateFreeAndCopy(uint8_t *mem, In_MemoryMeta *meta, void *src, size_t size)
{
    void *at = mem + meta->offset;
    memcpy(at, src, size);
    free(src);
    meta->offset += size;
    return at;
}

void Redex_LocalizeMemoryRecursive(In_MemoryMeta *meta, uint8_t *mem, Redex_Group *group)
{
    group->subgroups = In_AllocateFreeAndCopy(mem, meta, group->subgroups, group->subgroups_len * sizeof *group->subgroups);

    for (size_t i = 0; i < group->subgroups_len; ++i) {
        Redex_SubGroup *sub = &group->subgroups[i];

        switch (sub->type) {
            case Redex_SubGroup_Group:
                Redex_LocalizeMemoryRecursive(meta, mem, &sub->group);
                break;
            case Redex_SubGroup_Charset:
                sub->charset.ranges = In_AllocateFreeAndCopy(mem, meta, sub->charset.ranges, sub->charset.ranges_len * sizeof *sub->charset.ranges);
            default:
                break;
        }
    }
}

// Rearranges pointers so to localize them in RAM
void Redex_LocalizeMemory(In_Compiler *co, Redex_CompiledExpression *expr)
{
    expr->memory = malloc(sizeof(Redex_CharacterRange) * co->ranges_len + sizeof(Redex_SubGroup) * co->subgroups_len);
    Redex_LocalizeMemoryRecursive(&(In_MemoryMeta){0}, expr->memory, &expr->root);
}

Redex_CompiledExpression Redex_Compile(const char *redex)
{
    Redex_Group out = {0};
    In_Compiler co = In_InitCompiler(redex);

    while (In_IsCompiling(&co)) {
        In_AddSubgroup(&co, &out, In_CompileBasic(&co));
    }

    Redex_CompiledExpression expr_out = (Redex_CompiledExpression){out};
    Redex_LocalizeMemory(&co, &expr_out);

    return expr_out;
}

void Redex_CompiledExpressionDeinit(Redex_CompiledExpression *expr) {
    free(expr->memory);
}

