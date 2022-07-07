#include "Redex.h"
#include <assert.h>
#include <stdlib.h>

#define REALLOC_PERIOD 32

Redex_Match Redex_MatchGroup(BufferTape tape, Redex_Group *group);

static bool In_MatchSubgroup(BufferTape *tape, Redex_SubGroup *subgroup)
{
    switch (subgroup->type) {
        case Redex_SubGroup_Char: {
            // Must be handled by In_MatchCharFast class of functions
            assert(false);
        } break;
        case Redex_SubGroup_CharacterClass: {
            switch (subgroup->character_class) {
                case Redex_CharacterClass_Any: {
                    if (BufferTape_Get(tape) == 0) {
                        return false;
                    } 
                    BufferTape_Next(tape);
                } break;
                case Redex_CharacterClass_Count: assert(false); break;
            }
        } break;
        case Redex_SubGroup_Charset: {
            if (subgroup->charset.inverted) {
                int ch = BufferTape_Next(tape);
                for (size_t i = 0; i < subgroup->charset.ranges_len; ++i) {
                    Redex_CharacterRange range = subgroup->charset.ranges[i];

                    if (ch >= range.from && ch <= range.to) {
                        return false;
                    }
                }    
                return true;       
            } else {
                int ch = BufferTape_Next(tape);
                for (size_t i = 0; i < subgroup->charset.ranges_len; ++i) {
                    Redex_CharacterRange range = subgroup->charset.ranges[i];

                    if (ch >= range.from && ch <= range.to) {
                        return true;
                    }
                }
                return false;
            }

        } break;
        case Redex_SubGroup_Group: {
            Redex_Match match = Redex_MatchGroup(*tape, &subgroup->group);
            *tape = match.end;
            return match.success;
        } break;
        case Redex_SubGroup_Count: assert(false); break;
    }

    return true;
}

// Matches subgroup but retreats to original tape state if it fails 
// It also returns false if match was empty
static bool In_MatchSubgroupRetreat(BufferTape *tape, Redex_SubGroup *subgroup)
{
    BufferTape temp = *tape;
    if (In_MatchSubgroup(tape, subgroup) == false) {
        *tape = temp;
        return false;
    }
    return true && Buffer_CompareCursor(tape->cursor, temp.cursor) != 0;
}

// Matches a single character with * quantifier
static bool In_MatchCharFastAll(BufferTape *tape, uint32_t ch) 
{
    while (BufferTape_Get(tape) == ch) {
        BufferTape_Next(tape);
    }
    return true;
}

// Matches a single character with + quantifier
static bool In_MatchCharFastGreedy(BufferTape *tape, uint32_t ch) 
{
    if (BufferTape_Get(tape) != ch) {
        return false;
    }
    while (BufferTape_Get(tape) == ch) {
        BufferTape_Next(tape);
    }
    return true;
}

// Matches a single character with ? quantifier
static bool In_MatchCharFastLazy(BufferTape *tape, uint32_t ch) 
{
    if (BufferTape_Get(tape) == ch) {
        BufferTape_Next(tape);
    }
    return true;
}

// Matches a single character without quantifier
static bool In_MatchCharFastNone(BufferTape *tape, uint32_t ch) 
{
    return BufferTape_Next(tape) == ch;
}

static bool In_MatchQuant(BufferTape *tape, Redex_SubGroup *subgroup)
{
    switch (subgroup->quantifier) {
        case Redex_Quantifier_All: { // *
            switch (subgroup->type) {
                case Redex_SubGroup_Char: 
                    return In_MatchCharFastAll(tape, subgroup->ch);
                default:
                    while (In_MatchSubgroupRetreat(tape, subgroup))
                        ;
                    return true;
            }
        } break;
        case Redex_Quantifier_Greedy: { // +
            switch (subgroup->type) {
                case Redex_SubGroup_Char: 
                    return In_MatchCharFastGreedy(tape, subgroup->ch);
                default:
                    if (!In_MatchSubgroup(tape, subgroup)) {
                        return false;
                    }
                    while (In_MatchSubgroupRetreat(tape, subgroup))
                        ;
                    return true;
            }

        } break;
        case Redex_Quantifier_Lazy: { // ?
            switch (subgroup->type) {
                case Redex_SubGroup_Char: 
                    return In_MatchCharFastLazy(tape, subgroup->ch);
                default:
                    In_MatchSubgroupRetreat(tape, subgroup);
                    return true;
            }
        } break;
        case Redex_Quantifier_None: { // 
            switch (subgroup->type) {
                case Redex_SubGroup_Char: 
                    return In_MatchCharFastNone(tape, subgroup->ch);
                default:
                    return In_MatchSubgroup(tape, subgroup);
            }
        } break;
        case Redex_Quantifier_Count: assert(false); break;
    }
    assert(false);
}

Redex_Match Redex_MatchGroup(BufferTape tape, Redex_Group *group)
{
    Redex_Match match = (Redex_Match){.success = true, .end = tape};
    for (size_t i = 0; i < group->subgroups_len; ++i) {
        if (In_MatchQuant(&tape, &group->subgroups[i]) == false) {
            match.success = false;
            match.end = tape;
            return match;
        }
    }

    match.end = tape;
    return match;
}

Redex_Match Redex_GetMatch(BufferTape tape, Redex_CompiledExpression *expr)
{
    return Redex_MatchGroup(tape, &expr->root);
}