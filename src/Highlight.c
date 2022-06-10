#include "BufferDraw.h"
#include "Utf8.h"
#include "Redex.h"
#include "Highlight.h"
#include "Colors.h"

static bool In_ProcessWrapped(const char *begin, const char *end, const char *escape, BufferTape *tape)
{
    Redex_Match match = Redex_GetMatch(*tape, begin);
    
    // Prevent empty matches
    if (Buffer_CompareCursor(match.end.cursor, tape->cursor) == 0) {
        return false;
    }

    if (match.success) {
        *tape = match.end;
        while (BufferTape_Get(tape)) {
            if ((match = Redex_GetMatch(*tape, end)).success) {
                *tape = match.end;
                break;
            }
            
            if ((match = Redex_GetMatch(*tape, escape)).success && Buffer_CompareCursor(tape->cursor, match.end.cursor) != 0) {
                *tape = match.end;
            } else {
                BufferTape_Next(tape);
            }
        }
        return true;
    } else {
        return false;
    }
}

static bool In_ProcessLookahead(const char *data, const char *tail, BufferTape *tape)
{
    Redex_Match match = Redex_GetMatch(*tape, data);

    if (match.success) {
        *tape = match.end;
        return Redex_GetMatch(*tape, tail).success;
    } else {
        return false;
    }
}

static bool In_ProcessRedex(const char *redex, BufferTape *tape)
{
    Redex_Match match = Redex_GetMatch(*tape, redex);

    if (match.success) {
        *tape = match.end;
        return true;
    } else {
        return false;
    }
}

static bool In_ProcessAnyKw(const char **table, BufferTape *tape)
{
    // TODO: Binary search
    for (int i = 0; table[i]; i++) {
        BufferTape copy = *tape;

        // NOTE: This is a quick hack, ideally when I have nested rules I can have keyword derive from identifier
        if (In_ProcessRedex(table[i], &copy) && !isalnum(BufferTape_Get(&copy)) && BufferTape_Get(&copy) != '_') {
            *tape = copy;
            return true;
        }
    }
    return false;
}

void Highlight_HighlightBuffer(Buffer *buf, const Highlight_Set *set, BufferDrawSegments *out_segments)
{
    BufferDraw_InvalidateSegments(out_segments);
    BufferTape tape = BufferTape_Init(buf);
    BufferTape newTape;

    while (BufferTape_Get(&tape)) {
        SDL_Color color = Redactor_Color_Fore;

        bool match = false;
        for (int i = 0; i < set->rules_count; ++i)  {
            newTape = tape;
            Highlight_Rule *rule = &set->rules[i];
            switch (rule->rule_type) {
            case Highlight_Rule_Redex:
                match = In_ProcessRedex(rule->rule_redex, &newTape);
                break;
            case Highlight_Rule_Lookahead:
                match = In_ProcessLookahead(rule->rule_lookahead.data, rule->rule_lookahead.tail, &newTape);
                break;
            case Highlight_Rule_Wrapped:
                match = In_ProcessWrapped(rule->rule_wrapped.begin, rule->rule_wrapped.end, rule->rule_wrapped.slash, &newTape);
                break;
            case Highlight_Rule_AnyKw:
                match = In_ProcessAnyKw(rule->rule_anykw, &newTape);
                break;
            default: 
                break;
            }

            if (match) {
                color = rule->color;
                break;
            } 
        }

        if (match) {
            BufferDraw_InsertSegment(out_segments, tape.cursor.line, tape.cursor.column, BufferTape_GetSubstringMemoryOffset(&tape), Redactor_Color_Fore);
            BufferDraw_InsertSegment(out_segments, newTape.cursor.line, newTape.cursor.column, BufferTape_GetSubstringMemoryOffset(&newTape), color);
            tape = newTape;
        } else {
            BufferTape_Next(&tape);
        }
    }
    BufferDraw_InsertSegment(out_segments, tape.cursor.line, tape.cursor.column, BufferTape_GetSubstringMemoryOffset(&tape), Redactor_Color_Fore);
}

