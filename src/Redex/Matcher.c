#include "Redex.h"
#include "BufferTape.h"

#include <stdlib.h>

#define REALLOC_PERIOD 32

static bool In_MatchSubgroup(BufferTape *tape, Redex_SubGroup *subgroup)
{
    switch (subgroup->type) {
        case Redex_SubGroup_Char: {
            if (BufferTape_Get(tape) != subgroup->ch) {
                return false;
            }
            BufferTape_Next(tape);
        } break;
        case Redex_SubGroup_CharacterClass: {
            switch (subgroup->character_class) {
                case Redex_CharacterClass_Any: {
                    if (BufferTape_Get(tape) == 0) {
                        return false;
                    } 
                    BufferTape_Next(tape);
                } break;
            }
        } break;
        case Redex_SubGroup_Charset: {
            bool success = false;

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
            Redex_Match match = Redex_GetMatch(*tape, &subgroup->group);
            *tape = match.end;
            return match.success;
        } break;
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

static bool In_MatchQuant(BufferTape *tape, Redex_SubGroup *subgroup)
{
    switch (subgroup->quantifier) {
        case Redex_Quantifier_All: { // *
            while (In_MatchSubgroupRetreat(tape, subgroup))
                ;
            return true;
        } break;
        case Redex_Quantifier_Greedy: { // +
            if (!In_MatchSubgroup(tape, subgroup)) {
                return false;
            }
            while (In_MatchSubgroupRetreat(tape, subgroup))
                ;
            return true;
        } break;
        case Redex_Quantifier_Lazy: { // ?
            In_MatchSubgroupRetreat(tape, subgroup);
            return true;
        } break;
        case Redex_Quantifier_None: { // 
            return In_MatchSubgroup(tape, subgroup);
        } break;
    }
}

Redex_Match Redex_GetMatch(BufferTape tape, Redex_Group *group)
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